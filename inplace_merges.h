#pragma once

#include <algorithm>
#include <iostream>

#include "srt.h"

namespace v1 {

template <typename I, typename Compare>
void inplace_merge_rotating_middles(I f, I m, I l, Compare comp) {
  if (f == m || m == l) return;
  I left_m = srt::middle(f, m);
  I right_m = std::lower_bound(m, l, *left_m);
  m = std::rotate(left_m, m, right_m);

  if (f == left_m) return; // middle of one element is always that element.
  inplace_merge_rotating_middles(m, right_m, l, comp);
  inplace_merge_rotating_middles(f, left_m, m, comp);
}

}  // namespace v1

namespace v2 {

template <typename I, typename Compare>
void inplace_merge_with_space_for_rotate(I f,
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
  inplace_merge_with_space_for_rotate(m, right_m, l, comp, buf);
  inplace_merge_with_space_for_rotate(f, left_m, m, comp, buf);
}

template <typename I, typename Compare>
void inplace_merge_rotating_middles(I f, I m, I l, Compare comp) {
  srt::ibuffer<I> buf(std::min(std::distance(f, m), std::distance(m, l)));
  inplace_merge_with_space_for_rotate(f, m, l, comp, buf);
}

}  // namespace v2

