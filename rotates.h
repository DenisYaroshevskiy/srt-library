#pragma once

#include <algorithm>
#include <tuple>
#include <memory>
#include <iterator>
#include <cassert>
#include <tuple>

namespace detail {

template <typename I>
using ValueType = typename std::iterator_traits<I>::value_type;

template <typename I>
using DifferenceType = typename std::iterator_traits<I>::difference_type;

template <typename I>
using IteratorCategory = typename std::iterator_traits<I>::iterator_category;

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
  return std::copy(f, l, r);
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

  ~temporary_buffer() {
    while (buffer_ != end_) {
      end_->~T();
      --end_;
    }
    std::return_temporary_buffer(buffer_);
  }

private:
  T* buffer_;
  T* end_;
  std::ptrdiff_t count_;
};

}  // namespace detail


namespace v1 {

template <typename I>
I rotate_buffered_lhs(I f,
                      I m,
                      I l,
                      detail::temporary_buffer<detail::ValueType<I>>& buf) {
  detail::ValueType<I>* buf_f;
  detail::ValueType<I>* buf_l;

  std::tie(std::ignore, buf_f, buf_l) =
      buf.copy(std::move_iterator<I>(f), std::move_iterator<I>(m));
  I res = std::move(m, l, f);
  std::move(buf_f, buf_l, res);
  return res;
}

template <typename I>
I rotate_buffered_rhs(I f,
                      I m,
                      I l,
                      detail::temporary_buffer<detail::ValueType<I>>& buf) {
  detail::ValueType<I>* buf_f;
  detail::ValueType<I>* buf_l;

  std::tie(std::ignore, buf_f, buf_l) =
      buf.copy(std::move_iterator<I>(m), std::move_iterator<I>(l));
  I res = std::move_backward(f, m, l);
  std::move(buf_f, buf_l, f);
  return res;
}

template <typename I>
// requires RandomAccessIterator<I>
I rotate(I f, I m, I l) {
  detail::DifferenceType<I> lhs_size = std::distance(f, m);
  detail::DifferenceType<I> rhs_size = std::distance(m, l);
  using temp_buf = detail::temporary_buffer<detail::ValueType<I>>;

  if (lhs_size <= rhs_size) {
     temp_buf buf(lhs_size);
    if (buf.capacity() >= lhs_size) {
      return rotate_buffered_lhs(f, m, l, buf);
    }
  } else {
    temp_buf buf(rhs_size);
    if (buf.capacity() >= rhs_size) {
      return rotate_buffered_rhs(f, m, l, buf);
    }
  }

  return std::rotate(f, m, l);
}

}  // namespace v1
