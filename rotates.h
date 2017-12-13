#pragma once

#include <algorithm>
#include <tuple>
#include <memory>
#include <iterator>
#include <cassert>
#include <tuple>

#include "srt.h"

namespace v1 {

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

template <typename I>
// requires RandomAccessIterator<I>
I rotate_with_enough_space(I f, I m, I l, srt::ibuffer<I>& buf) {
  srt::DifferenceType<I> lhs_size = std::distance(f, m);
  srt::DifferenceType<I> rhs_size = std::distance(m, l);

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

template <typename I>
I rotate(I f, I m, I l) {
  srt::ibuffer<I> buf(std::min(std::distance(f, m), std::distance(m, l)));
  return rotate_with_enough_space(f, m, l, buf);
}

}  // namespace v1
