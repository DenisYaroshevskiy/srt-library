#include <numeric>
#include <vector>

#include "benchmark/benchmark.h"

constexpr size_t kSize = 1000;
using value_type = int;

template <typename I, typename O, typename P>
// require ForwardIterator<I> && ForwardIterator<O> && BinaryPredicate<P>
O copy_until_adjacent_check_fwd(I f, I l, O o, P p) {
  if (f == l) return f;
  *o++ = *f++;
  while (f != l && p(*o, *f))
    *o++ = *f++;
  return o;
}

template <typename I, typename O, typename P>
// require ForwardIterator<I> && OutputIterator<O> && BinaryPredicate<P>
O copy_until_adjacent_check(I f, I l, O o, P p) {
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

template <typename I, typename O>
O copy_until_sorted_fwd(I f, I l, O o) {
  return copy_until_adjacent_check_fwd(f, l, o, std::less<>{});
}

template <typename I, typename O>
O copy_until_sorted(I f, I l, O o) {
  return copy_until_adjacent_check(f, l, o, std::less<>{});
}

std::vector<value_type> generate_input() {
  std::vector<value_type> res(kSize);
  std::iota(res.begin(), res.end(), 0);
  return res;
}

void plain_copy(benchmark::State& state) {
  auto input = generate_input();
  for (auto _ : state) {
    auto copy = input;
    benchmark::DoNotOptimize(copy);
  }
}

void is_sorted(benchmark::State& state) {
  auto input = generate_input();
  for (auto _ : state) {
    benchmark::DoNotOptimize(std::is_sorted(input.begin(), input.end()));
    auto copy(input);
    benchmark::DoNotOptimize(copy);
  }
}

void copy_until_sorted_default_construct_fwd(benchmark::State& state) {
  auto input = generate_input();
  for (auto _ : state) {
    std::vector<value_type> copy(input.size());
    copy_until_sorted_fwd(input.begin(), input.end(), copy.begin());
  }
}

void copy_until_sorted_default_construct(benchmark::State& state) {
  auto input = generate_input();
  for (auto _ : state) {
    std::vector<value_type> copy(input.size());
    copy_until_sorted(input.begin(), input.end(), copy.begin());
  }
}

void copy_until_sorted_back_inseter(benchmark::State& state) {
  auto input = generate_input();
  for (auto _ : state) {
    std::vector<value_type> copy;
    copy.reserve(input.size());
    copy_until_sorted(input.begin(), input.end(), std::back_inserter(copy));
  }
}

BENCHMARK(plain_copy);
BENCHMARK(is_sorted);
BENCHMARK(copy_until_sorted_default_construct_fwd);
BENCHMARK(copy_until_sorted_default_construct);
BENCHMARK(copy_until_sorted_back_inseter);

BENCHMARK_MAIN();


