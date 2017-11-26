#include <algorithm>
#include <limits>

namespace v1 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  for (; f1 != l1; ++o) {
    if (f2 == l2)
      return std::copy(f1, l1, o);
    if (comp(*f2, *f1)) {
      *o = *f2;
      ++f2;
    } else {
      *o = *f1;
      if (!comp(*f1, *f2)) // libc++ bug.
        ++f2;
      ++f1;
    }
  }
  return std::copy(f2, l2, o);
}

}  // namespace v1

namespace v2 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  for (; f1 != l1; ++o) {
    if (f2 == l2)
      return std::copy(f1, l1, o);
    if (comp(*f2, *f1)) {
      *o = *f2;
      ++f2;
    } else {
      if (!comp(*f1, *f2))
        ++f2;
      *o = *f1;
      ++f1;
    }
  }
  return std::copy(f2, l2, o);
}

}  // namespace v2

namespace v3 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (comp(*f2, *f1)) {
      *o++ = *f2++;
      if (f2 == l2) goto copyFirst;
    } else {
      if (!comp(*f1, *f2)) {
        ++f2;
        if (f2 == l2) goto copyFirst;
      }
      *o++ = *f1++;
      if (f1 == l1) goto copySecond;
    }
  }

 copySecond:
  return std::copy(f2, l2, o);
 copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v3

namespace v4 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (comp(*f1, *f2)) {
      *o++ = *f1++;
      if (f1 == l1) goto copySecond;
    } else {
      if (comp(*f2, *f1)) *o++ = *f2;
      ++f2; if (f2 == l2) goto copyFirst;
    }
  }

 copySecond:
  return std::copy(f2, l2, o);
 copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v4

namespace v5 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (__builtin_expect(comp(*f1, *f2), true)) {
      *o++ = *f1++;
      if (f1 == l1) goto copySecond;
    } else {
      if (comp(*f2, *f1)) *o++ = *f2;
      ++f2; if (f2 == l2) goto copyFirst;
    }
  }

 copySecond:
  return std::copy(f2, l2, o);
 copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v5

namespace v6 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;
  goto start;

 checkSecond:
  if (comp(*f2, *f1)) *o++ = *f2;
  ++f2; if (f2 == l2) goto copyFirst;

 start:
  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;
  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;
  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;
  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;
  goto start;

 copySecond:
  return std::copy(f2, l2, o);
 copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v6

namespace v7 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
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
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v7

namespace v8 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  I1 next_f1 = f1;
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;
  goto start;

 checkSecond:
  if (comp(*f2, *f1)) *o++ = *f2;
  ++f2; if (f2 == l2) goto copyFirst;

 start:
  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;
  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;
  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;
  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;

  next_f1 = std::lower_bound(f1, l1, *f2, comp);
  o = std::copy(f1, next_f1, o);
  f1 = next_f1; if (f1 == l1) goto copySecond;
  goto checkSecond;

 copySecond:
  return std::copy(f2, l2, o);
 copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v8

namespace v9 {

template <typename I, typename P>
I partition_point_biased(I f, I l, P p) {
  auto len = std::distance(f, l);
  int step = 1;
  while (len > step) {
    I test = std::next(f, step);
    if (!p(*test)) {
      l = test;
      break;
    }
    f = ++test;
    len -= step + 1;
    step += step;
  }
  return std::partition_point(f, l, p);
}

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  I1 next_f1 = f1;
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;
  goto start;

 checkSecond:
  if (comp(*f2, *f1)) *o++ = *f2;
  ++f2; if (f2 == l2) goto copyFirst;

 start:
  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;
  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;
  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;
  if (!comp(*f1, *f2)) goto checkSecond;
  *o++ = *f1++; if (f1 == l1) goto copySecond;

  next_f1 = partition_point_biased(f1, l1,
                                   [&](const auto& x) { return comp(x, *f2); });
  o = std::copy(f1, next_f1, o);
  f1 = next_f1; if (f1 == l1) goto copySecond;
  goto checkSecond;

 copySecond:
  return std::copy(f2, l2, o);
 copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v9

namespace v10 {

template <typename I>
using DifferenceType = typename std::iterator_traits<I>::difference_type;

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

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
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
        find_boundary(f1, l1, [&](const auto& x) { return comp(x, *f2); });
    o = std::copy(f1, segment_end, o);
    f1 = segment_end;
  }

 copySecond:
  return std::copy(f2, l2, o);
 copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v10

