// ---------------------------------------------------------------------------
// srt library ---------------------------------------------------------------
// ---------------------------------------------------------------------------

#ifndef SRT_LIBRARY_H_
#define SRT_LIBRARY_H_

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <vector>

namespace srt {

namespace detail {

template <typename...>
using void_t = void;

template <typename, typename = void>
struct has_is_transparent_member : std::false_type {};

template <typename T>
struct has_is_transparent_member<T, void_t<typename T::is_transparent>>
    : std::true_type {};

}  // namespace detail

// meta functions -------------------------------------------------------------

template <typename C>
using Iterator = typename C::iterator;

template <typename I>
using ValueType = typename std::iterator_traits<I>::value_type;

template <typename I>
using Reference = typename std::iterator_traits<I>::reference;

template <typename I>
using DifferenceType = typename std::iterator_traits<I>::difference_type;

template <typename I>
using IteratorCategory = typename std::iterator_traits<I>::iterator_category;

// concepts ------------------------------------------------------------------

template <typename I>
constexpr bool RandomAccessIterator() {
  return std::is_same<IteratorCategory<I>,
                      std::random_access_iterator_tag>::value;
}

template <typename T>
constexpr bool TransparentComparator() {
  return detail::has_is_transparent_member<T>::value;
}

// predeclarations ------------------------------------------------------------

struct less;

template <typename I>
class temporary_buffer;

template <typename I>
using ibuffer = temporary_buffer<ValueType<I>>;

template <typename I, typename O>
// requires InputIterator<I> && OutputIterator<O>
O copy(I f, I l, O o);

template <typename I, typename O, typename P>
// requires ForwardIterator<I> && OutputIterator<O> ||
//          InputIterator<I> && ForwardIterator<O>
O copy_until_adjacent_check(I f, I l, O o, P p);

template <typename I, typename O>
// requires ForwardIterator<I> && OutputIterator<O> ||
//          InputIterator<I> && ForwardIterator<O>
O copy_until_sorted(I f, I l, O o);

template <typename I, typename O>
// requires InputIterator<I> && OutputIterator<O>
O uninitialized_copy(I f, I l, O o);

template <typename I>
// requires RandomAccessIterator<I>
I middle(I f, I l);

template <typename I, typename Compare>
// requires RandomAccessIterator<I> && StrictWeakOrdering<Compare<ValueType<I>>
I sort_and_unique(I f, I l, Compare comp);

template <typename I1, typename I2, typename O, typename Compare>
// requires ForwardIterator<I> && StrictWeakOrdering<Compare<ValueType<I>>
O set_union_linear(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp);

template <typename I1, typename I2, typename O, typename Compare>
// requires ForwardIterator<I>
O set_union_linear(I1 f1, I1 l1, I2 f2, I2 l2, O o);

template <typename I1, typename I2, typename O, typename Compare>
// requires RandomAccessIterator<I> && StrictWeakOrdering<Compare<ValueType<I>>
O set_union_biased(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp);

template <typename I1, typename I2, typename O, typename Compare>
// requires RandomAccessIterator<I>
O set_union_biased(I1 f1, I1 l1, I2 f2, I2 l2, O o);

template <typename I, typename P>
// requires RandomAccessIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_biased(I f, I l, P p);

template <typename I, typename P>
// requires RandomAccessIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_hinted(I f, I hint, I l, P p);

template <typename I, typename V, typename Compare>
// requires RandomAccessIterator<I> && StrictWeakOrdering<Compare<ValueType<I>>
I lower_bound_biased(I f, I l, V v, Compare comp);

template <typename I, typename V>
// requires RandomAccessIterator<I>
I lower_bound_biased(I f, I l, V v);

template <typename I, typename V, typename Compare>
// requires RandomAccessIterator<I> && StrictWeakOrdering<Compare<ValueType<I>>
I lower_bound_hinted(I f, I hint, I l, V v, Compare comp);

template <typename I, typename V>
// requires RandomAccessIterator<I>
I lower_bound_hinted(I f, I hint, I l, V v);

template <typename I>
// requires RandomAccessIterator<I>
I rotate_buffered(I f, I m, I l, ibuffer<I>& buf);

template <typename I>
// requires RandomAccessIterator<I>
I rotate_buffered(I f, I m, I l);

template <typename I, typename Compare>
// requires RandomAccessIterator<I> && StrictWeakOrdering<Compare<ValueType<I>>
void inplace_merge_rotating_middles(I f, I m, I l, Compare comp);

template <typename I, typename Compare>
// requires RandomAccessIterator<I>
void inplace_merge_rotating_middles(I f, I m, I l);

template <typename I, typename Compare>
// requires RandomAccessIterator<I> && StrictWeakOrdering<Compare<ValueType<I>>
void inplace_merge_rotating_middles_buffered(I f,
                                             I m,
                                             I l,
                                             Compare comp,
                                             srt::ibuffer<I>& buf);

template <typename I>
// requires RandomAccessIterator<I>
void inplace_merge_rotating_middles_buffered(I f,
                                             I m,
                                             I l,
                                             srt::ibuffer<I>& buf);

template <typename I, typename Compare>
// requires RandomAccessIterator<I> && StrictWeakOrdering<Compare<ValueType<I>>
void inplace_merge_rotating_middles_buffered(I f, I m, I l, Compare comp);


template <typename I>
// requires RandomAccessIterator<I>
void inplace_merge_rotating_middles_buffered(I f, I m, I l);

// implementation -------------------------------------------------------------

namespace detail {

template <typename F>
// requires Predicate<F>
struct not_fn_t {
  F f;

  template <typename... Args>
  bool operator()(Args&&... args) {
    return !f(std::forward<Args>(args)...);
  }
};

template <typename F>
struct inverse_t {
  F f;

  template <typename X, typename Y>
  bool operator()(const X& x, const Y& y) {
    return f(y, x);
  }
};

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

// clang-format off
template <typename ContainerValueType, typename InsertedType>
using insert_should_be_enabled =
typename std::enable_if
<
  std::is_same<
    ContainerValueType,
    typename std::remove_cv<
      typename std::remove_reference<InsertedType>::type
    >::type
  >::value
>::type;
// clang-format on

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

template <typename I, typename P>
I find_boundary(I f, I l, P p) {
  I sent = middle(f, l);
  if (p(*sent)) return sent;
  return partition_point_biased_no_checks(f, p);
}

template <class I1, class I2, class O, class Comp>
std::tuple<I1, I2, O> set_union_intersecting_parts(I1 f1,
                                                   I1 l1,
                                                   I2 f2,
                                                   I2 l2,
                                                   O o,
                                                   Comp comp) {
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

    I1 segment_end =
        find_boundary(f1, l1, [&](Reference<I1> x) { return comp(x, *f2); });
    o = srt::copy(f1, segment_end, o);
    f1 = segment_end;
  }

 copySecond:
 copyFirst:
  return std::make_tuple(f1, f2, o);
}

template <typename I1, typename I2, typename P>
// requires ForwardIterator<I1> && ForwardIterator<I2> &&
//          StrictWeakOrdering<P, ValueType<I>>
std::pair<I1, I1> set_union_into_tail(I1 buf, I1 f1, I1 l1, I2 f2, I2 l2, P p) {
  std::move_iterator<I1> move_f1;
  std::tie(move_f1, f2, buf) =
      set_union_intersecting_parts(std::make_move_iterator(f1),  //
                                   std::make_move_iterator(l1),  //
                                   f2, l2,                       //
                                   buf, p);                      //

  return {srt::copy(f2, l2, buf), move_f1.base()};
}

template <typename C, typename I, typename P>
// requires  Container<C> &&  ForwardIterator<I> &&
//           StrictWeakOrdering<P(ValueType<C>)>
void insert_first_last_impl(C& c, I f, I l, P p) {
  auto new_len = std::distance(f, l);
  auto orig_len = c.size();
  c.resize(orig_len + 2 * new_len);

  Iterator<C> orig_f = c.begin();
  Iterator<C> orig_l = c.begin() + orig_len;
  Iterator<C> f_in = c.end() - new_len;
  Iterator<C> l_in = c.end();
  Iterator<C> buf = f_in;

  srt::copy(f, l, f_in);
  l_in = sort_and_unique(f_in, l_in, p);

  using reverse_it = typename C::reverse_iterator;
  auto move_reverse_it =
      [](Iterator<C> it) { return std::make_move_iterator(reverse_it(it)); };

  auto reverse_remainig_buf_range = detail::set_union_into_tail(
      reverse_it(buf), reverse_it(orig_l), reverse_it(orig_f),
      move_reverse_it(l_in), move_reverse_it(f_in), inverse_fn(p));

  auto remaining_buf =
      std::make_pair(reverse_remainig_buf_range.second.base() - c.begin(),
                     reverse_remainig_buf_range.first.base() - c.begin());

  c.erase(c.end() - new_len, c.end());
  c.erase(c.begin() + remaining_buf.first, c.begin() + remaining_buf.second);
}

template <typename I, typename O>
constexpr bool enable_trivial_copy() {
   return std::is_trivially_copy_constructible<ValueType<O>>::value &&
          std::is_same<typename std::remove_const<ValueType<I>>::type, ValueType<O>>::value;
}

template <typename I, typename O>
typename std::enable_if<!detail::enable_trivial_copy<I, O>(), O>::type
do_uninitialized_copy(I f, I l, O o)
{
    using value_type = typename std::iterator_traits<I>::value_type;
    O s = o;
    try
    {
        for (; f != l; ++f, (void) ++o)
            ::new (static_cast<void*>(std::addressof(*o))) value_type(*f);
    }
    catch (...)
    {
        for (; s != o; ++s)
            s->~value_type();
        throw;
    }
    return o;
}

template <typename I, typename O>
typename std::enable_if<detail::enable_trivial_copy<I, O>(), O>::type
do_uninitialized_copy(I f, I l, O o) {
  return srt::copy(f, l, o);
}

template <typename I, typename O, typename P>
O do_copy_until_adjacent_check(I f, I l, O o, P p, std::output_iterator_tag) {
  if (f == l) return o;

  I next = f;
  ++next;
  for (; next != l; ++next, ++f) {
    if (!p(*f, *next)) break;
    *o = *f; ++o;
  }

  *o = *f; ++o;
  return o;
}

template <typename I, typename O, typename P>
O do_copy_until_adjacent_check(I f, I l, O o, P p, std::forward_iterator_tag) {
  if (f == l) return o;

  I next = f;
  ++next;
  for (; next != l; ++next, ++f) {
    if (!p(*f, *next)) break;
    *o = *f; ++o;
  }
  *o = *f; ++o;
  return o;
}

template <typename I>
I rotate_buffered_lhs(I f, I m, I l, srt::ibuffer<I>& buf) {
  srt::ValueType<I>* buf_f;
  srt::ValueType<I>* buf_l;

  std::tie(std::ignore, buf_f, buf_l) =
      buf.copy(std::move_iterator<I>(f), std::move_iterator<I>(m));
  I res = std::move(m, l, f);
  std::move(buf_f, buf_l, res);
  return res;
}

template <typename I>
I rotate_buffered_rhs(I f, I m, I l, srt::ibuffer<I>& buf) {
  srt::ValueType<I>* buf_f;
  srt::ValueType<I>* buf_l;

  std::tie(std::ignore, buf_f, buf_l) =
      buf.copy(std::move_iterator<I>(m), std::move_iterator<I>(l));
  I res = std::move_backward(f, m, l);
  std::move(buf_f, buf_l, f);
  return res;
}



}  // namespace detail

// temporary_buffer -----------------------------------------------------------

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
    T* res_end = srt::uninitialized_copy(f, l, res_begin);
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

// functors -------------------------------------------------------------------

struct less {
  template <typename X, typename Y>
  bool operator()(const X& x, const Y& y) noexcept(noexcept(x < y)) {
    return x < y;
  }

  using is_transparent = int;
};

template <typename F>
// requires Predicate<F>
detail::not_fn_t<F> not_fn(F f) noexcept {
  return {f};
}

template <typename F>
detail::inverse_t<F> inverse_fn(F f) noexcept {
  return {f};
}

// algorithms -----------------------------------------------------------------

template <typename I, typename O>
O copy(I f, I l, O o) {
  return detail::do_copy<false>(f, l, o);
}

template <typename I>
I middle(I f, I l) {
  static_assert(
    std::numeric_limits<DifferenceType<I>>::max() <=
    std::numeric_limits<size_t>::max(),
    "iterators difference type is too big");
  return std::next(f, static_cast<size_t>(std::distance(f, l)) / 2);
}

template <typename I, typename O>
O uninitialized_copy(I f, I l, O o) {
  return detail::do_uninitialized_copy(f, l, o);
}

// Think: stable_sort is a merge sort. Merge can be replaced with set_union ->
// unique would not be required. Quick sort is not modified that easily (is it?)
// to do this. How much does the unique matter? For the 1000 elements - log is
// 10 - unique is 1 => 1/10? Measuring this would be cool.

template <typename I, typename Compare>
I sort_and_unique(I f, I l, Compare comp) {
  std::sort(f, l, comp);
  return std::unique(f, l, not_fn(comp));
}

template <typename I>
I sort_and_unique(I f, I l) {
  return sort_and_unique(f, l, less{});
}

template <typename I1, typename I2, typename O, typename Compare>
O set_union_linear(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp) {
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
  }

copySecond:
  return srt::copy(f2, l2, o);
copyFirst:
  return srt::copy(f1, l1, o);
}

template <typename I1, typename I2, typename O>
O set_union_linear(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
  return set_union_linear(f1, l1, f2, l2, o, less{});
}

template <typename I1, typename I2, typename O, typename Compare>
O set_union_biased(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp) {
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
        f1, l1, [&](Reference<I1> x) { return comp(x, *f2); });
    o = srt::copy(f1, segment_end, o);
    f1 = segment_end;
  }

 copySecond:
  return srt::copy(f2, l2, o);
 copyFirst:
  return srt::copy(f1, l1, o);
}

template <typename I1, typename I2, typename O>
O set_union_biased(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
  return set_union_biased(f1, l1, f2, l2, o, less{});
}

template <typename I, typename O, typename P>
O copy_until_adjacent_check(I f, I l, O o, P p) {
  return detail::do_copy_until_adjacent_check(f, l, o, p,
                                              IteratorCategory<O>{});
}

template <typename I, typename O>
O copy_until_sorted(I f, I l, O o) {
  return copy_until_adjacent_check(f, l, o, less{});
}

template <typename I, typename P>
I partition_point_biased(I f, I l, P p) {
  while (f != l) {
    I sent = srt::middle(f, l);
    if (!p(*sent)) return detail::partition_point_biased_no_checks(f, p);
    f = ++sent;
  }
  return f;
}

template <typename I, typename P>
I partition_point_hinted(I f, I hint, I l, P p) {
  I rhs = partition_point_biased(hint, l, p);
  if (rhs != hint)
    return rhs;

  return partition_point_biased(std::reverse_iterator<I>(hint),
                                std::reverse_iterator<I>(f), not_fn(p)).base();
}

template <typename I, typename V, typename Compare>
I lower_bound_biased(I f, I l, V v, Compare comp) {
  return partition_point_biased(f, l,
                                [&](Reference<I> x) { return comp(x, v); });
}

template <typename I, typename V>
I lower_bound_biased(I f, I l, V v) {
  return lower_bound_biased(f, l, v, less{});
}

template <typename I, typename V, typename Compare>
I lower_bound_hinted(I f, I hint, I l, V v, Compare comp) {
  return partition_point_hinted(f, hint, l,
                                [&](Reference<I> x) { return comp(x, v); });
}

template <typename I, typename V>
I lower_bound_hinted(I f, I hint, I l, V v) {
  return lower_bound_hinted(f, hint, l, v, less{});
}

template <typename I>
I rotate_buffered(I f, I m, I l, ibuffer<I>& buf) {
  srt::DifferenceType<I> lhs_size = std::distance(f, m);
  srt::DifferenceType<I> rhs_size = std::distance(m, l);

  if (lhs_size <= rhs_size) {
    if (buf.capacity() >= lhs_size) {
      return detail::rotate_buffered_lhs(f, m, l, buf);
    }
  } else {
    if (buf.capacity() >= rhs_size) {
      return detail::rotate_buffered_rhs(f, m, l, buf);
    }
  }

  return std::rotate(f, m, l);
}

template <typename I>
I rotate_buffered(I f, I m, I l) {
  srt::ibuffer<I> buf(std::min(std::distance(f, m), std::distance(m, l)));
  return rotate_buffered(f, m, l, buf);
}

template <typename I, typename Compare>
void inplace_merge_rotating_middles(I f, I m, I l, Compare comp) {
  if (f == m || m == l) return;
  I left_m = middle(f, m);
  I right_m = std::lower_bound(m, l, *left_m);
  m = std::rotate(left_m, m, right_m);

  if (f == left_m) return; // middle of one element is always that element.
  inplace_merge_rotating_middles(m, right_m, l, comp);
  inplace_merge_rotating_middles(f, left_m, m, comp);
}

template <typename I>
void inplace_merge_rotating_middles(I f, I m, I l) {
  inplace_merge_rotating_middles(f, m, l, less{});
}

template <typename I, typename Compare>
void inplace_merge_rotating_middles_buffered(I f,
                                             I m,
                                             I l,
                                             Compare comp,
                                             srt::ibuffer<I>& buf) {
  if (f == m || m == l) return;
  I left_m = srt::middle(f, m);
  I right_m = std::lower_bound(m, l, *left_m);
  m = srt::rotate_buffered(left_m, m, right_m, buf);
  buf.clear();

  if (f == left_m) return; // middle of one element is always that element.
  inplace_merge_rotating_middles_buffered(m, right_m, l, comp, buf);
  inplace_merge_rotating_middles_buffered(f, left_m, m, comp, buf);
}

template <typename I>
void inplace_merge_rotating_middles_buffered(I f,
                                             I m,
                                             I l,
                                             srt::ibuffer<I>& buf) {
  return inplace_merge_rotating_middles_buffered(f, m, l, less{}, buf);
}

template <typename I, typename Compare>
void inplace_merge_rotating_middles_buffered(I f, I m, I l, Compare comp) {
  srt::ibuffer<I> buf(std::min(std::distance(f, m), std::distance(m, l)));
  inplace_merge_rotating_middles_buffered(f, m, l, comp, buf);
}

template <typename I>
void inplace_merge_rotating_middles_buffered(I f, I m, I l) {
  inplace_merge_rotating_middles_buffered(f, m, l, less{});
}

// vector ---------------------------------------------------------------------
// std::vector's api is not rich enough to implement flat_sets on top of it
// in the most efficient way - we have to have access to the capacity.
// This implementation of vector supports that.
//
// Note: we do not support the vector<bool> specialization. flat_set<bool>.

template <typename T, typename Alloc = std::allocator<T>>
// requires SemiRegular<T> && Allocator<Alloc>
class vector {
  using alloc_traits = std::allocator_traits<Alloc>;
 public:
  using value_type = T;
  using allocator_type = Alloc;
  using reference = value_type&;
  using const_reference = const value_type&;
  using size_type = typename alloc_traits::size_type;
  using difference_type = typename alloc_traits::difference_type;
  using pointer = typename alloc_traits::pointer;
  using const_pointer = typename alloc_traits::const_pointer;
  using iterator = pointer;
  using const_iterator = const_pointer;

 private:
  struct impl_t : allocator_type
  {
    impl_t() = default;
    impl_t(const allocator_type& a) : allocator_type(a) {}
    pointer begin_ = nullptr;
    pointer end_ = nullptr;
    pointer buffer_end_ = nullptr;
  } impl_;

  allocator_type& alloc() noexcept { return impl_; }
  const allocator_type& alloc() const noexcept { return impl_; }

 public:
  allocator_type get_allocator() const noexcept {
    return alloc();
  }

  iterator begin() noexcept { return impl_.begin_; }
  const_iterator begin() const noexcept { return impl_.begin_; }
  const_iterator cbegin() const noexcept { return begin(); }

  iterator end() noexcept { return impl_.end_; }
  const_iterator end() const noexcept { return impl_.end_; }
  const_iterator cend() const noexcept { return end(); }

  iterator buffer_end() noexcept { return impl_.buffer_end_; }
  const_iterator buffer_end() const noexcept { return impl_.buffer_end_;}
  const_iterator cbuffer_end() const noexcept { return buffer_end();}

  size_type capacity() const noexcept { return buffer_end() - begin(); }
  void clear() noexcept {
    while (begin() != impl_.end_) {
      alloc_traits::destroy(alloc(), &*--impl_.end_);
    }
  }

  vector() noexcept(
      std::is_nothrow_default_constructible<allocator_type>::value) {}
  vector(const allocator_type& a) : impl_(a) {}
  ~vector() {
    clear();
    alloc_traits::deallocate(alloc(), begin(), capacity());
  }
};

// flat_set -------------------------------------------------------------------

template <typename Key,
          typename Compare = less,
          typename UnderlyingType = std::vector<Key>>
// requires (todo)
class flat_set {
 public:
  using underlying_type = UnderlyingType;
  using key_type = Key;
  using value_type = key_type;
  using size_type = typename underlying_type::size_type;
  using difference_type = typename underlying_type::difference_type;
  using key_compare = Compare;
  using value_compare = Compare;
  using reference = typename underlying_type::reference;
  using const_reference = typename underlying_type::const_reference;
  using pointer = typename underlying_type::pointer;
  using const_pointer = typename underlying_type::const_pointer;
  using iterator = typename underlying_type::iterator;
  using const_iterator = typename underlying_type::const_iterator;
  using reverse_iterator = typename underlying_type::reverse_iterator;
  using const_reverse_iterator =
      typename underlying_type::const_reverse_iterator;

 private:
  // We cannot use std::tuple for compressed pair, because there is no way to
  // forward many arguments to one of the members.
  struct impl_t : value_compare {
    impl_t() = default;

    template <typename... Args>
    impl_t(value_compare comp, Args&&... args)
        : value_compare(comp), body_{std::forward<Args>(args)...} {};

    underlying_type body_;
  }
  impl_;

  template <typename V>
  using type_for_value_compare =
      typename std::conditional<TransparentComparator<value_compare>(),
                                V,
                                value_type>::type;

  iterator const_cast_iterator(const_iterator c_it) {
    return begin() + std::distance(cbegin(), c_it);
  }

 public:
  // --------------------------------------------------------------------------
  // Lifetime -----------------------------------------------------------------

  flat_set() = default;
  explicit flat_set(const key_compare& comp) : impl_{comp} {}

  template <typename I>
  // requires InputIterator<I>
  flat_set(I f, I l, const key_compare& comp = key_compare())
      : impl_(comp, f, l) {
    erase(sort_and_unique(begin(), end(), key_compare()), end());
  }

  flat_set(const flat_set&) = default;
  flat_set(flat_set&&) = default;

  explicit flat_set(underlying_type buf,
                    const key_compare& comp = key_compare())
      : impl_{comp, std::move(buf)} {
    erase(sort_and_unique(begin(), end(), key_compare()), end());
  }

  flat_set(std::initializer_list<value_type> il,
           const key_compare& comp = key_compare())
      : flat_set(il.begin(), il.end(), comp) {}

  ~flat_set() = default;

  // --------------------------------------------------------------------------
  // Assignments --------------------------------------------------------------

  flat_set& operator=(const flat_set&) = default;
  flat_set& operator=(flat_set&&) = default;
  flat_set& operator=(std::initializer_list<value_type> il) {
    body() = il;
    erase(sort_and_unique(begin(), end(), key_compare()), end());
    return *this;
  }

  //---------------------------------------------------------------------------
  // Memory management.

  void reserve(size_type new_capacity) { body().reserve(new_capacity); }
  size_type capacity() const { return body().capacity(); }
  void shrink_to_fit() { body().shrink_to_fit(); }

  //---------------------------------------------------------------------------
  // Size management.

  void clear() { body().clear(); }

  size_type size() const { return body().size(); }
  size_type max_size() const { return body().max_size(); }

  bool empty() const { return body().empty(); }

  //---------------------------------------------------------------------------
  // Iterators.

  iterator begin() { return body().begin(); }
  const_iterator begin() const { return body().begin(); }
  const_iterator cbegin() const { return body().cbegin(); }

  iterator end() { return body().end(); }
  const_iterator end() const { return body().end(); }
  const_iterator cend() const { return body().cend(); }

  reverse_iterator rbegin() { return body().rbegin(); }
  const_reverse_iterator rbegin() const { return body().rbegin(); }
  const_reverse_iterator crbegin() const { return body().crbegin(); }

  reverse_iterator rend() { return body().rend(); }
  const_reverse_iterator rend() const { return body().rend(); }
  const_reverse_iterator crend() const { return body().crend(); }

  //---------------------------------------------------------------------------
  // Insert operations.

  template <typename V,
            typename = detail::insert_should_be_enabled<value_type, V>>
  std::pair<iterator, bool> insert(V&& v) {
    iterator pos = lower_bound(v);
    if (pos == end() || value_comp()(v, *pos))
      return {body().insert(pos, std::forward<V>(v)), true};
    return {pos, false};
  }

  template <typename V,
            typename = detail::insert_should_be_enabled<value_type, V>>
  iterator insert(const_iterator hint, V&& v) {
    auto pos = lower_bound_hinted(cbegin(), hint, cend(), v, value_comp());
    if (pos == end() || value_comp()(v, *pos))
      return body().insert(pos, std::forward<V>(v));
    return const_cast_iterator(pos);
  }

  template <typename I>
  void insert(I f, I l) {
    detail::insert_first_last_impl(body(), f, l, value_comp());
  }

  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    return insert(value_type{std::forward<Args>(args)...});
  }

  template <typename... Args>
  iterator emplace_hint(const_iterator hint, Args&&... args) {
    return insert(hint, value_type{std::forward<Args>(args)...});
  }

  // --------------------------------------------------------------------------
  // Erase operations.

  iterator erase(iterator pos) { return body().erase(pos); }
  iterator erase(const_iterator pos) { return body().erase(pos); }

  iterator erase(const_iterator f, const_iterator l) {
    return body().erase(f, l);
  }

  template <typename V>
  size_type erase(const V& v) {
    auto eq_range = equal_range(v);
    size_type res = std::distance(eq_range.first, eq_range.second);
    erase(eq_range.first, eq_range.second);
    return res;
  }

  // --------------------------------------------------------------------------
  // Search operations.

  template <typename V>
  size_type count(const V& v) const {
    auto eq_range = equal_range(v);
    return std::distance(eq_range.first, eq_range.second);
  }

  template <typename V>
  iterator find(const V& v) {
    auto eq_range = equal_range(v);
    return (eq_range.first == eq_range.second) ? end() : eq_range.first;
  }

  template <typename V>
  const_iterator find(const V& v) const {
    auto eq_range = equal_range(v);
    return (eq_range.first == eq_range.second) ? end() : eq_range.first;
  }

  template <typename V>
  std::pair<iterator, iterator> equal_range(const V& v) {
    auto pos = lower_bound(v);
    if (pos == end() || value_comp()(v, *pos))
      return {pos, pos};

    return {pos, std::next(pos)};
  }

  template <typename V>
  std::pair<const_iterator, const_iterator> equal_range(const V& v) const {
    auto pos = lower_bound(v);
    if (pos == end() || value_comp()(v, *pos))
      return {pos, pos};

    return {pos, std::next(pos)};
  }

  template <typename V>
  iterator lower_bound(const V& v) {
    const type_for_value_compare<V>& v_ref = v;
    return std::lower_bound(begin(), end(), v_ref, value_comp());
  }

  template <typename V>
  const_iterator lower_bound(const V& v) const {
    const type_for_value_compare<V>& v_ref = v;
    return std::lower_bound(begin(), end(), v_ref, value_comp());
  }

  template <typename V>
  iterator upper_bound(const V& v) {
    const type_for_value_compare<V>& v_ref = v;
    return std::upper_bound(begin(), end(), v_ref, value_comp());
  }

  template <typename V>
  const_iterator upper_bound(const V& v) const {
    const type_for_value_compare<V>& v_ref = v;
    return std::upper_bound(begin(), end(), v_ref, value_comp());
  }

  //---------------------------------------------------------------------------
  // Getters.

  key_compare key_comp() const { return impl_; }
  value_compare value_comp() const { return impl_; }

  underlying_type& body() { return impl_.body_; }
  const underlying_type& body() const { return impl_.body_; }

  //---------------------------------------------------------------------------
  // General operations.

  void swap(flat_set& x) { body().swap(x.body()); }

  friend void swap(flat_set& x, flat_set& y) { x.swap(y); }

  friend bool operator==(const flat_set& x, const flat_set& y) {
    return x.body() == y.body();
  }

  friend bool operator!=(const flat_set& x, const flat_set& y) {
    return !(x == y);
  }

  friend bool operator<(const flat_set& x, const flat_set& y) {
    return x.body() < y.body();
  }

  friend bool operator>(const flat_set& x, const flat_set& y) { return y < x; }

  friend bool operator<=(const flat_set& x, const flat_set& y) {
    return !(y < x);
  }

  friend bool operator>=(const flat_set& x, const flat_set& y) {
    return !(x < y);
  }
};

template <typename Key,
          typename Comparator,
          typename UnderlyingType,
          typename P>
// requires UnaryPredicate<P(reference)>
void erase_if(flat_set<Key, Comparator, UnderlyingType>& x, P p) {
  x.erase(std::remove_if(x.begin(), x.end(), p), x.end());
}

}  // namespace srt

#endif  // SRT_LIBRARY_H_

// ---------------------------------------------------------------------------
// srt library ---------------------------------------------------------------
// ---------------------------------------------------------------------------
