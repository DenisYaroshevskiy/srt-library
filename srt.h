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

template <typename C>
using ContainerValueType = typename C::value_type;

template <typename C>
using ContainerSizeType = typename C::size_type;

// concepts ------------------------------------------------------------------

template <typename I>
constexpr bool ForwardIterator() {
  return std::is_base_of<std::forward_iterator_tag, IteratorCategory<I>>::value;
}

template <typename I>
constexpr bool RandomAccessIterator() {
  return std::is_same<IteratorCategory<I>,
                      std::random_access_iterator_tag>::value;
}

template <typename T>
constexpr bool TransparentComparator() {
  return detail::has_is_transparent_member<T>::value;
}

template <typename C>
constexpr bool ResizeableContainer() {
  return std::is_trivially_default_constructible<ContainerValueType<C>>::value;
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
O set_union_unique_linear(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp);

template <typename I1, typename I2, typename O, typename Compare>
// requires ForwardIterator<I>
O set_union_unique_linear(I1 f1, I1 l1, I2 f2, I2 l2, O o);

template <typename I1, typename I2, typename O, typename Compare>
// requires RandomAccessIterator<I> && StrictWeakOrdering<Compare<ValueType<I>>
O set_union_unique_biased(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp);

template <typename I1, typename I2, typename O, typename Compare>
// requires RandomAccessIterator<I>
O set_union_unique_biased(I1 f1, I1 l1, I2 f2, I2 l2, O o);

template <typename I, typename N, typename P>
// requires ForwardIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_n(I f, DifferenceType<I> n, P p);

template <typename I, typename P>
// requires ForwardIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point(I f, I l, P p);

template <typename I, typename V, typename Compare>
// requires ForwardIterator<I> && StrictWeakOrdering<Compare<ValueType<I>>
I lower_bound(I f, I l, const V& v, Compare comp);

template <typename I, typename V>
// requires RandomAccessIterator<I>
I lower_bound(I f, I l, const V& v);

template <typename I, typename P>
// requires RandomAccessIterator<I> && UnaryPredicate<P, ValueType<I>>  // TODO:
// copy support for ForwardIterator
I partition_point_biased(I f, I l, P p);

template <typename I, typename P>
// requires RandomAccessIterator<I> && UnaryPredicate<P, ValueType<I>>
I partition_point_hinted(I f, I hint, I l, P p);

template <typename I, typename V, typename Compare>
// requires RandomAccessIterator<I> && StrictWeakOrdering<Compare<ValueType<I>>
I lower_bound_biased(I f, I l, V v, Compare comp);

template <typename I, typename V>
// requires RandomAccessIterator<I>  // TODO: pass by const ref.
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
void inplace_merge_rotating_middles_buffered(I f, I m, I l, Compare comp,
                                             srt::ibuffer<I>& buf);

template <typename I>
// requires RandomAccessIterator<I>
void inplace_merge_rotating_middles_buffered(I f, I m, I l,
                                             srt::ibuffer<I>& buf);

template <typename I, typename Compare>
// requires RandomAccessIterator<I> && StrictWeakOrdering<Compare<ValueType<I>>
void inplace_merge_rotating_middles_buffered(I f, I m, I l, Compare comp);

template <typename I>
// requires RandomAccessIterator<I>
void inplace_merge_rotating_middles_buffered(I f, I m, I l);

template <typename C, typename T>
// requires RandomAccessContainer<C> && std::is_same<ContainerValueType<C>, T>
void resize_with_junk(C& c, T&& sample, ContainerSizeType<C> new_len);

// functors -------------------------------------------------------------------

struct less;

namespace detail {

template <typename F>
struct not_fn_t;

template <typename F>
struct inverse_t;

}  // namespace detail

template <typename F>
// requires Predicate<F>
detail::not_fn_t<F> not_fn(F f) noexcept;

template <typename F>
detail::inverse_t<F> inverse_fn(F f) noexcept;

// implementation -------------------------------------------------------------

namespace detail {

template <typename T>
class junk_iterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using reference = T&&;
  using pointer = T*;
  using iterator_category = std::random_access_iterator_tag;

  junk_iterator(pointer junk_sample, difference_type count)
      : junk_sample_{junk_sample}, count_{count} {}

  reference operator*() const { return std::move(*junk_sample_); }

  junk_iterator& operator++() {
    ++count_;
    return *this;
  }
  junk_iterator operator++(int) {
    junk_iterator tmp = *this;
    operator++();
    return tmp;
  }

  junk_iterator& operator+=(difference_type distance) {
    count_ += distance;
    return *this;
  }

  friend junk_iterator operator+(const junk_iterator& x,
                                 difference_type distance) {
    junk_iterator tmp = x;
    tmp += distance;
    return tmp;
  }

  friend difference_type operator-(const junk_iterator& x,
                                   const junk_iterator& y) {
    return x.count_ - y.count_;
  }

  friend bool operator==(const junk_iterator& x, const junk_iterator& y) {
    return x.count_ == y.count_;
  }

  friend bool operator!=(const junk_iterator& x, const junk_iterator& y) {
    return !(x == y);
  }

 private:
  pointer junk_sample_;
  difference_type count_;
};

template <typename T>
std::pair<junk_iterator<T>, junk_iterator<T>> junk_range(T& junk_sample,
                                                         std::ptrdiff_t size) {
  return {junk_iterator<T>{&junk_sample, 0},
          junk_iterator<T>{&junk_sample, size}};
}

template <typename C>
typename std::enable_if<ResizeableContainer<C>(), void>::type
do_resize_with_junk(C& c, ContainerValueType<C>&,
                    ContainerSizeType<C> new_len) {
  c.resize(new_len);
}

template <typename C>
// requires Container<C>
typename std::enable_if<!ResizeableContainer<C>(), void>::type
do_resize_with_junk(C& c, ContainerValueType<C>& sample,
                    ContainerSizeType<C> new_len) {
  if (new_len <= c.size()) {
    c.erase(std::prev(c.end(), c.size() - new_len), c.end());
    return;
  }

  ContainerValueType<C> junk_sample = std::move(sample);
  sample = std::move(junk_sample);
  auto junk = junk_range(junk_sample, new_len - c.size());
  c.insert(c.end(), junk.first, junk.second);
}

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

// libc++ currently does not copy optimize to memmove for reverse iterators.
// This is a small hack to fix this.
// A proper patch has been submitted: https://reviews.llvm.org/D38653 but it was
// rejected.

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
      return O(
          do_copy<is_backward>(unwrap<condition>(f), unwrap<condition>(l), o));
  }

  {
    constexpr bool condition =
        is_reverse_iterator<I>::value && is_reverse_iterator<O>::value;
    constexpr bool new_is_backward = condition ? !is_backward : is_backward;
    if (condition)
      return O(do_copy<new_is_backward>(
          unwrap<condition>(l), unwrap<condition>(f), unwrap<condition>(o)));
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

// clang-format off
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
// clang-format on

template <typename I, typename P>
I find_boundary(I f, I l, P p) {
  I sent = middle(f, l);
  if (p(*sent)) return sent;
  return partition_point_biased_no_checks(f, p);
}

// clang-format off
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
// clang-format on

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

template <typename I>
std::reverse_iterator<I> make_reverse_iterator(I it) {
  return std::reverse_iterator<I>{it};
}

template <typename C, typename I, typename P>
// requires Container<C> && ForwardIterator<I> &&
// StrictWeakOrdering<P(ValueType<C>)>
void insert_sorted_unique_impl(C& c, I f, I l, P p) {
  auto new_len = std::distance(f, l);
  auto orig_len = c.size();

  resize_with_junk(c, *f, orig_len + new_len);

  Iterator<C> orig_f = c.begin();
  Iterator<C> orig_l = c.begin() + orig_len;

  auto reverse_remainig_buf_range = detail::set_union_into_tail(
      detail::make_reverse_iterator(c.end()),
      detail::make_reverse_iterator(orig_l),
      detail::make_reverse_iterator(orig_f), detail::make_reverse_iterator(l),
      detail::make_reverse_iterator(f), inverse_fn(p));

  c.erase(reverse_remainig_buf_range.second.base(),
          reverse_remainig_buf_range.first.base());
}

template <typename I, typename O>
constexpr bool enable_trivial_copy() {
  return std::is_trivially_copy_constructible<ValueType<O>>::value &&
         std::is_same<typename std::remove_const<ValueType<I>>::type,
                      ValueType<O>>::value;
}

template <typename I, typename O>
typename std::enable_if<!detail::enable_trivial_copy<I, O>(), O>::type
do_uninitialized_copy(I f, I l, O o) {
  using value_type = typename std::iterator_traits<I>::value_type;
  O s = o;
  try {
    for (; f != l; ++f, (void)++o)
      ::new (static_cast<void*>(std::addressof(*o))) value_type(*f);
  } catch (...) {
    for (; s != o; ++s) s->~value_type();
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
    *o = *f;
    ++o;
  }

  *o = *f;
  ++o;
  return o;
}

template <typename I, typename O, typename P>
O do_copy_until_adjacent_check(I f, I l, O o, P p, std::forward_iterator_tag) {
  if (f == l) return o;

  I next = f;
  ++next;
  for (; next != l; ++next, ++f) {
    if (!p(*f, *next)) break;
    *o = *f;
    ++o;
  }
  *o = *f;
  ++o;
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

template <typename N>
typename std::enable_if<std::is_integral<N>::value, N>::type  //
half_positive(N n) {
  using UnsignedN = typename std::make_unsigned<N>::type;
  return static_cast<N>(static_cast<UnsignedN>(n) / 2);
}

template <typename N>
typename std::enable_if<!std::is_integral<N>::value, N>::type  //
half_positive(N n) {
  return n / 2;
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
  static_assert(std::numeric_limits<DifferenceType<I>>::max() <=
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

// clang-format off
template <typename I1, typename I2, typename O, typename Compare>
O set_union_unique_linear(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp) {
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
// clang-format on

template <typename I1, typename I2, typename O>
O set_union_unique_linear(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
  return set_union_unique_linear(f1, l1, f2, l2, o, less{});
}

// clang-format off
template <typename I1, typename I2, typename O, typename Compare>
O set_union_unique_biased(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp) {
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
// clang-format on

template <typename I1, typename I2, typename O>
O set_union_unique_biased(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
  return set_union_unique_biased(f1, l1, f2, l2, o, less{});
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
I partition_point_n(I f, DifferenceType<I> n, P p) {
  while (n) {
    DifferenceType<I> n2 = detail::half_positive(n);
    I m = std::next(f, n2);
    if (p(*m)) {
      f = ++m;
      n -= n2 + 1;
    } else {
      n = n2;
    }
  }
  return f;
}

template <typename I, typename P>
I partition_point(I f, I l, P p) {
  return srt::partition_point_n(f, std::distance(f, l), p);
}

template <typename I, typename V, typename Compare>
I lower_bound(I f, I l, const V& v, Compare comp) {
  return srt::partition_point(f, l, [&](Reference<I> x) { return comp(x, v); });
}

template <typename I, typename V>
I lower_bound(I f, I l, const V& v) {
  return srt::lower_bound(f, l, v, srt::less{});
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
  if (rhs != hint) return rhs;

  return partition_point_biased(std::reverse_iterator<I>(hint),
                                std::reverse_iterator<I>(f), not_fn(p))
      .base();
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

  if (f == left_m) return;  // middle of one element is always that element.
  inplace_merge_rotating_middles(m, right_m, l, comp);
  inplace_merge_rotating_middles(f, left_m, m, comp);
}

template <typename I>
void inplace_merge_rotating_middles(I f, I m, I l) {
  inplace_merge_rotating_middles(f, m, l, less{});
}

template <typename I, typename Compare>
void inplace_merge_rotating_middles_buffered(I f, I m, I l, Compare comp,
                                             srt::ibuffer<I>& buf) {
  if (f == m || m == l) return;
  I left_m = srt::middle(f, m);
  I right_m = std::lower_bound(m, l, *left_m);
  m = srt::rotate_buffered(left_m, m, right_m, buf);
  buf.clear();

  if (f == left_m) return;  // middle of one element is always that element.
  inplace_merge_rotating_middles_buffered(m, right_m, l, comp, buf);
  inplace_merge_rotating_middles_buffered(f, left_m, m, comp, buf);
}

template <typename I>
void inplace_merge_rotating_middles_buffered(I f, I m, I l,
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

template <typename C, typename T>
void resize_with_junk(C& c, T&& sample, ContainerSizeType<C> new_len) {
  detail::do_resize_with_junk(c, sample, new_len);
}

// flat_set -------------------------------------------------------------------

template <typename Key, typename Compare = less,
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
  } impl_;

  template <typename V>
  using type_for_value_compare =
      typename std::conditional<TransparentComparator<value_compare>(), V,
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
  void insert_sorted_unique(I f, I l) {
    // Need to count elements.
    if (!ForwardIterator<I>()) {
      underlying_type buf(f, l, body().get_allocator());
      insert_sorted_unique(std::make_move_iterator(buf.begin()),
                           std::make_move_iterator(buf.end()));
      return;
    }

    detail::insert_sorted_unique_impl(body(), f, l, value_comp());
  }

  template <typename I>
  void insert(I f, I l) {
    underlying_type buf(f, l, body().get_allocator());
    buf.erase(sort_and_unique(buf.begin(), buf.end(), value_comp()), buf.end());
    insert_sorted_unique(std::make_move_iterator(buf.begin()),
                         std::make_move_iterator(buf.end()));
  }

  void insert(std::initializer_list<value_type> ilist) {
    insert(ilist.begin(), ilist.end());
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
    if (pos == end() || value_comp()(v, *pos)) return {pos, pos};

    return {pos, std::next(pos)};
  }

  template <typename V>
  std::pair<const_iterator, const_iterator> equal_range(const V& v) const {
    auto pos = lower_bound(v);
    if (pos == end() || value_comp()(v, *pos)) return {pos, pos};

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

template <typename Key, typename Comparator, typename UnderlyingType,
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
