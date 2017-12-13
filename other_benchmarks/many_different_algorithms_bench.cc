#include <algorithm>
#include <numeric>
#include <random>
#include <set>
#include <vector>

#include "benchmark/benchmark.h"

namespace detail {

template <typename I>
using ValueType = typename std::iterator_traits<I>::value_type;

template <typename I>
using DifferenceType = typename std::iterator_traits<I>::difference_type;

template <typename I>
using IteratorCategory = typename std::iterator_traits<I>::iterator_category;

// copy --------------------------------------------------------------------

// libc++ currently does not optimize to memmove for reverse iterators.
// This is a small hack to fix this.
// A proper patch has been submitted: https://reviews.llvm.org/D38653

template <typename I>
struct is_reverse_iterator : std::false_type {};

template <typename I>
struct is_reverse_iterator<std::reverse_iterator<I>> : std::true_type {};

template <typename I>
struct is_move_iterator : std::false_type {};

template <typename I>
struct is_move_iterator<std::move_iterator<I>> : std::true_type {};

// clang-format off
template <bool condition, typename I>
typename std::enable_if<!condition, I>::type
unwrap(I it) {
  assert(false);
  return it;
}
// clang-format on

// clang-format off
template <bool condition, typename I>
typename std::enable_if<condition, typename I::iterator_type>::type
unwrap(I it) {
  return it.base();
}
// clang-format on

template <bool is_backward, typename I, typename O>
typename std::enable_if<!is_backward, O>::type call_copy(I f, I l, O o) {
  return std::copy(f, l, o);
}

template <bool is_backward, typename I, typename O>
typename std::enable_if<is_backward, O>::type call_copy(I f, I l, O o) {
  return std::copy_backward(f, l, o);
}

template <bool is_backward, typename I, typename O>
O do_copy(I f, I l, O o) {
  constexpr bool is_trivial_copy =
      std::is_trivially_copy_assignable<ValueType<O>>::value &&
      std::is_same<typename std::remove_const<ValueType<I>>::type,
                   ValueType<O>>::value;

  {
    constexpr bool condition = is_move_iterator<I>::value && is_trivial_copy;
    if (condition)
      return O(do_copy<is_backward>(unwrap<condition>(f),
                                    unwrap<condition>(l),
                                    o));
  }

  {
    constexpr bool condition =
        is_reverse_iterator<I>::value && is_reverse_iterator<O>::value;
    constexpr bool new_is_backward = condition ? !is_backward : is_backward;
    if (condition)
      return O(do_copy<new_is_backward>(unwrap<condition>(l),
                                        unwrap<condition>(f),
                                        unwrap<condition>(o)));
  }
  return O(call_copy<is_backward>(f, l, o));
}

template <typename I, typename O>
O copy(I f, I l, O o) {
  return do_copy<false>(f, l, o);
}

// temporary buffer -----------------------------------------------------------

template <typename I>
constexpr bool RandomAccessIterator() {
  return std::is_same<IteratorCategory<I>,
                      std::random_access_iterator_tag>::value;
}

template <typename I, typename O>
constexpr bool enable_trivial_copy() {
   return std::is_trivially_copy_constructible<ValueType<O>>::value &&
          std::is_same<typename std::remove_const<ValueType<I>>::type, ValueType<O>>::value;
}

template <class I, class O>
typename std::enable_if<!enable_trivial_copy<I, O>(), O>::type
uninitialized_copy(I f, I l, O r)
{
    using value_type = typename std::iterator_traits<I>::value_type;
    O s = r;
    try
    {
        for (; f != l; ++f, (void) ++r)
            ::new (static_cast<void*>(std::addressof(*r))) value_type(*f);
    }
    catch (...)
    {
        for (; s != r; ++s)
            s->~value_type();
        throw;
    }
    return r;
}

template <class I, class O>
typename std::enable_if<enable_trivial_copy<I, O>(), O>::type
uninitialized_copy(I f, I l, O r) {
  return detail::copy(f, l, r);
}

template <typename T>
class temporary_buffer {
 public:
  temporary_buffer(std::ptrdiff_t count) {
    std::tie(buffer_, count_) = std::get_temporary_buffer<T>(count);
    end_ = buffer_;
  }

  temporary_buffer(temporary_buffer&& x) = delete;
  temporary_buffer& operator=(temporary_buffer&& x) = delete;

  std::ptrdiff_t capacity() const { return count_; }

  template <typename I>
  std::tuple<I, T*, T*> copy(I f, I l) {
    static_assert(RandomAccessIterator<I>(), "");
    T* res_begin = end_;
    if (std::distance(f, l) > count_) {
      l = std::next(f, count_);
    }
    T* res_end = detail::uninitialized_copy(f, l, res_begin);
    count_ -= res_begin - res_end;
    end_ = res_end;
    return std::make_tuple(l, res_begin, res_end);
  }

  void clear() {
    count_ += end_ - buffer_;
    while (buffer_ != end_) {
      end_->~T();
      --end_;
    }
  }

  ~temporary_buffer() {
    clear();
    std::return_temporary_buffer(buffer_);
  }

private:
  T* buffer_;
  T* end_;
  std::ptrdiff_t count_;
};

template <typename I>
using buffer = temporary_buffer<ValueType<I>>;

template <typename I>
I rotate_buffered_lhs(I f, I m, I l, buffer<I>& buf) {
  ValueType<I>* buf_f;
  ValueType<I>* buf_l;

  std::tie(std::ignore, buf_f, buf_l) =
      buf.copy(std::move_iterator<I>(f), std::move_iterator<I>(m));
  I res = std::move(m, l, f);
  std::move(buf_f, buf_l, res);
  return res;
}

template <typename I>
I rotate_buffered_rhs(I f, I m, I l, buffer<I>& buf) {
  ValueType<I>* buf_f;
  ValueType<I>* buf_l;

  std::tie(std::ignore, buf_f, buf_l) =
      buf.copy(std::move_iterator<I>(m), std::move_iterator<I>(l));
  I res = std::move_backward(f, m, l);
  std::move(buf_f, buf_l, f);
  return res;
}

template <typename I>
// requires RandomAccessIterator<I>
I rotate_with_enough_space(I f, I m, I l, buffer<I>& buf) {
  DifferenceType<I> lhs_size = std::distance(f, m);
  DifferenceType<I> rhs_size = std::distance(m, l);

  if (lhs_size <= rhs_size) {
    if (buf.capacity() >= lhs_size) {
      return rotate_buffered_lhs(f, m, l, buf);
    }
  } else {
    if (buf.capacity() >= rhs_size) {
      return rotate_buffered_rhs(f, m, l, buf);
    }
  }

  return std::rotate(f, m, l);
}

template <typename I, typename P>
I partition_point_biased_no_checks(I f, P p) {
  while(true) {
    if (!p(*f)) return f; ++f;
    if (!p(*f)) return f;  ++f;
    if (!p(*f)) return f;  ++f;
    for (DifferenceType<I> step = 2;; step += step) {
      I test = std::next(f, step);
      if (!p(*test)) break;
      f = ++test;
    }
  }
}

template <typename I>
I middle(I f, I l) {
  static_assert(
    std::numeric_limits<DifferenceType<I>>::max() <=
    std::numeric_limits<size_t>::max(),
    "iterators difference type is too big");
  return std::next(f, static_cast<size_t>(std::distance(f, l)) / 2);
}

template <typename I, typename P>
I find_boundary(I f, I l, P p) {
  I sent = middle(f, l);
  if (p(*sent)) return sent;
  return partition_point_biased_no_checks(f, p);
}

}  // namespace detail

using value_type = int;

constexpr size_t kProblemSize = 1000;
constexpr size_t kInsertingElementsSize = 10;

value_type random_number() {
  static std::mt19937 g;
  static std::uniform_int_distribution<> dis(1, int(kProblemSize) * 100);
  return dis(g);
}

auto generate_problem_set() {
  std::set<value_type> res;
  while (res.size() < kProblemSize)
    res.insert(random_number());
  return std::vector<value_type>(res.begin(), res.end());
}

auto generate_elements_for_insertion() {
  std::vector<value_type> vec(kInsertingElementsSize);
  std::generate(vec.begin(), vec.end(), random_number);
  return vec;
}

template <typename I, typename Compare>
void inplace_merge_rotating_middles(I f, I m, I l, Compare comp) {
  if (f == m || m == l) return;
  I left_m = detail::middle(f, m);
  I right_m = std::lower_bound(m, l, *left_m);
  m = std::rotate(left_m, m, right_m);

  if (f == left_m) return; // middle of one element is always that element.
  inplace_merge_rotating_middles(m, right_m, l, comp);
  inplace_merge_rotating_middles(f, left_m, m, comp);
}


template <typename I, typename Compare>
void inplace_merge_with_space_for_rotate(I f,
                                         I m,
                                         I l,
                                         Compare comp,
                                         detail::buffer<I>& buf) {
  if (f == m || m == l) return;
  I left_m = detail::middle(f, m);
  I right_m = std::lower_bound(m, l, *left_m);
  m = detail::rotate_with_enough_space(left_m, m, right_m, buf);
  buf.clear();

  if (f == left_m) return; // middle of one element is always that element.
  inplace_merge_with_space_for_rotate(m, right_m, l, comp, buf);
  inplace_merge_with_space_for_rotate(f, left_m, m, comp, buf);
}

template <class I1, class I2, class O, class Comp>
O set_union_biased(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    goto biased;

   checkSecond:
    if (comp(*f2, *f1)) *o++ = *f2;
    ++f2; if (f2 == l2) goto copyFirst;

   biased:
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;

    I1 segment_end = detail::find_boundary(
        f1, l1, [&](const auto& x) { return comp(x, *f2); });
    o = detail::copy(f1, segment_end, o);
    f1 = segment_end;
  }

 copySecond:
  return detail::copy(f2, l2, o);
 copyFirst:
  return detail::copy(f1, l1, o);
}


// benchmarks -------------------------------------------------------------

void inserting_by_one(benchmark::State& state) {
  auto problem_set = generate_problem_set();
  auto elements = generate_elements_for_insertion();
  for (auto _ : state) {
    auto copy = problem_set;
    for (auto e : elements) {
      auto pos = std::lower_bound(copy.begin(), copy.end(), e);
      if (pos == copy.end() || *pos != e)
        copy.insert(pos, e);
    }
  }
}

void unique(benchmark::State& state) {
  auto problem_set = generate_problem_set();
  auto elements = generate_elements_for_insertion();
  problem_set.insert(problem_set.end(), elements.begin(), elements.end());
  std::sort(problem_set.begin(), problem_set.end());
  for (auto _ : state) {
    auto copy = problem_set;
    copy.erase(std::unique(copy.begin(), copy.end()), copy.end());
  }
}

template <typename Alg>
void inplace_merge(benchmark::State& state) {
  auto problem_set = generate_problem_set();
  auto elements = generate_elements_for_insertion();
  problem_set.insert(problem_set.end(), elements.begin(), elements.end());
  std::sort(problem_set.begin() + kProblemSize, problem_set.end());
  for (auto _ : state) {
    auto copy = problem_set;
    Alg()(copy.begin(), copy.begin() + kProblemSize, copy.end());
  }
}

struct using_set_union_biased {
  template <typename I>
  void operator()(I f, I m, I l) {
    std::vector<detail::ValueType<I>> buf(m, l);
    using r_it = std::reverse_iterator<I>;
    set_union_biased(r_it(m), r_it(f),
                     r_it(buf.rbegin()), r_it(buf.rend()),
                     r_it(l), std::greater<>{});
  }
};

struct naive_rotating_middles {
  template <typename I>
  void operator()(I f, I m, I l) {
    inplace_merge_rotating_middles(f, m, l, std::less<>{});
  }
};

struct naive_rotating_middles_with_space_for_rotate
{
  template <typename I>
  void operator()(I f, I m, I l) {
    detail::buffer<I> buf(std::min(std::distance(f, m), std::distance(m, l)));
    inplace_merge_with_space_for_rotate(f, m, l, std::less<>{}, buf);
  }
};

struct std_inplace_merge {
  template <typename I>
  void operator()(I f, I m, I l) {
    std::inplace_merge(f, m , l);
  }
};

BENCHMARK(inserting_by_one);
BENCHMARK(unique);
BENCHMARK_TEMPLATE(inplace_merge, using_set_union_biased);
BENCHMARK_TEMPLATE(inplace_merge, naive_rotating_middles);
BENCHMARK_TEMPLATE(inplace_merge, naive_rotating_middles_with_space_for_rotate);
BENCHMARK_TEMPLATE(inplace_merge, std_inplace_merge);

BENCHMARK_MAIN();
