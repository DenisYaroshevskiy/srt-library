### Preface

# Goal.

This paper main goal is to discuss algorithms that go into implementing flat_set and different consequences that come out of those algorithms.

# flat_set.

By flat_set we would understand an associative set based on top of a vector like
container.

# Background.

The authors experience is based on implementing and using base::flat_set/base::flat_map from Chromium, analyzing existing implementations from Boost/Eastl/Folly, as well as doing a research srt library, that has the fastest performance on author's measurements.

# Assumptions.

It's the author's believe that we can implement two algorithms:

## partition_point_biased

Signature:
```cpp
template <typename I, typename P>
  requires ForwardIterator<I> && Predicate<P(ValueType<I>)>
I partition_point_biased(I f, I l, P p);
```cpp

The result is the same as for std::partition_point but unlike the last one, it
finds result significantly faster for elements closer to f at the expense of
possibly taking longer for further elements.

We also assume related algorithms, derived from this one:

// TODO: look up proper concepts requirements.

```cpp
template <typename I, typename V, typename Compare>
  requires ForwardIterator<I> && StrictWeakOrdering<Compare(ValuetType<I>, V)>
I lower_bound_biased(I f, I l, const V& v, Compare comp);
I upper_bound_biased(I f, I l, const V& v, Compare comp);
```cpp

## set_union_biased:

Signature:
```cpp
template <typename I1, typename I2, typename O, typename Compare>
 requires ForwardIterator<I1> && ForwardIterator<I2> &&
          OutputIterator<O> && StrictWeakOrdering<Compare>
O set_union_biased(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp);
```cpp

Result is the same as for std::set_union but it priorities range [f1, l1) over
[f2, l2). It can perform reasonably well both in cases when
std::distance(f1, l1)is much bigger than std::distance(f2, l2) and when they are
roughly the same.
It does so by combining linear merge (where we always compare each element,
like std::set_union) and falling back to using partition_point_biased in order
to skip many contiguous comparisons with the same result.

We also assume related algorithms:
```cpp

template <typename I1, typename I2, typename O, typename Compare>
 requires ForwardIterator<I1> && ForwardIterator<I2> &&
          OutputIterator<O> && StrictWeakOrdering<Compare>

O set_union_biased_second(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp);
O set_union_unbalanced(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp);

O merge_biased(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp);
O merge_biased_second(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp);
O merge_unbalanced(I1 f1, I1 l1, I2 f2, I2 l2, O o, Compare comp);
```cpp

Where
  *_second - is the same algorithm, but it biased to the range [f2, l2).
             It's a variation were we are biased to the side from which we drop
             elements.
  *_unbalanced - is the same algorithm, in the simplest version it just checks
                 whether [f1, l1) or [f2, l2) is bigger and does biased to that
                 side. The author thinks it's possible to do a better job at
                 this algorithm: it might be better at skipping elements from
                 both sides. However, it might have consequences both for
                 very unbalanced and very balanced cases.
  merge_* - similar variations on std::merge.

Information about possible implementation, measurements and tray-doffs can be
found in appendix.

### Design issues.

# Underlying type customization/access.

The real question I want to address here is whether or not to allow to customize
the underlying type.
Or, if not customize - at the very least have some access to it.

Customization:

Upside:
 People often write their own vectors for some reason, we could support them
 for free.
 Static sets are really important.
 I don't believe we can achieve everything by only allowing allocator customization.

Downside:
  flat_set needs to access the capacity of the underlying container and there
  is no current standard way to do that. (See bulk insertion section).

Access to the underlying type:

Upside:

(1) Fill the buffer - construct in bulk.

  Originally flat_set/flat_map in chromium didn't support construction from a
    moved buffer (like so):
  ```cpp
  flat_set<Widget> build_widget_set() {
    std::vector<Widget> widget_set_buffer; // underlying_type = std::vector
    // fill widget set buffer.
    return flat_set<Widget>(std::move(widget_set_buffer));
  }
  ```cpp

  And suggested to use range construction instead.

  The author had seen at the very least two different patches attempting to do this.
  (The second one was accepted). C++ developers really dislike extra copies/moves. And for a decent reason (todo measurements).

  In order to support this use case we have to give people a way to declare a
  type for a buffer.

(2) "Unsafe" operations.

   There are certain operations, that people would like to disable when
   using flat_containers. Primarily people that I talked to want const keys and
   don't want "unchecked" insertions.

   This is really bad for performance an limits visibility.

   There are plenty of good algorithms that become unusable/unimplementable
   without being a friend of flat_set/flat_map.

   Consider:
    * insert values without checks
    * std::erase_if
    * std::unique
    * std::unique(rbegin(), rend());
    * increase all keys by a fix value
    ...

   I heard suggestions to provide some of them as members, but I don't think that will work, because there are actually unlimited number of those.

   We have to be able to access underlying buffer.

   Possible interface:

   ```cpp
    flat_set(unsafe_t, underlying_type elements);
    underlying_type take_elements() &&;
   ```cpp

   If you need to get unsafe access - you still the buffer - do what you have to
   and then move it back with a special tag type.


Downisde:
  (1) In order to implement insert(first, last) (see below) flat_set needs a hook
  into capacity of the underlying type. As of c++17 interface of std::vector does
  not have that.

  (2) flat_set in some cases cannot use the growth factor of the underlying container
  (see insert(first, last) - we need extra space). It leads to some complex
  guaranties on methods like reserve().


Possible solutions:
  (1) We figure out another unsafe interface and buffered construction,
      that does not allow to access underlying buffer (we really have to
      provide them though).

  (2) We say that underlying type is always vector. Than the relationship between
      std::flat_set, std::vector is all in "implementation details", and there
      are no weired interfaces.

  (3) We allow for underlying type customization. Then we have to specify and
      commit to a bunch of new interfaces for std::vector.

# Stability.

  There are a few methods that are concerned with stability:

  range construction.
  range insertion.
  merge.

  With range construction it really affects the performance - whether stability
  is a requirement. Same goes for stability among newly inserted elements.

  However, I couldn't see a big benefit in loosing stability between old and new
  elements: we can save about 200 instructions in binary size and nothing
  in speed - doesn't seem worth an extra interface decision.

  In Chromium we use a stable_sort, because of flat_maps.
  Plus constructors and range insertion accept an extra parametr:
  KEEP_FIRST_OF_DUPS, KEEP_LAST_OF_DUPS - which of duplicates should be dropped.
  The variation with KEEP_LAST_OF_DUPS have a couple of important cases, for
  example they are used in jsons, because this is how json works.

  Possible solutions:
  To solve range stability we can either have:
    stable_* - versions of range algorithms
    stable_flat_set.

  To solve duplicate dependency, we can probably have a range version of
  insert_or_assign method.

  I don't think it's ok to always sort sets in a stable manner, because it's
  a big performance overhead. I would like to use flat_sets everywhere where I'd
  now use a sorted vector - because it's better and safer. I don't want any
  overhead.

# Bulk insertion.

  The best I know how to implement bulk insertion requires set_union_biased algorithm and access to the underlying capacity.

  It works in a similar fashion to std::inplace_merge, when it can get access to the buffer.


# Parallel keys/values for flat_map.





