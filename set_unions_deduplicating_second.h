#pragma once

#include <algorithm>

#include "srt.h"

namespace v1 {

template <typename I1, typename I2, typename O, typename Compare>
O set_union_deduplicating_second(I1 f1,
                                 I1 l1,
                                 I2 f2,
                                 I2 l2,
                                 O o,
                                 Compare comp) {
  l2 = std::unique(f2, l2, srt::not_fn(comp));
  return srt::set_union_biased(f1, l1, f2, l2, o, comp);
}

}  // namespace v1


namespace v2 {

}  // namespace v2
