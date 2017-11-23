#include <algorithm>
#include <functional>
#include <type_traits>
#include <iterator>

#include "benchmark/benchmark.h"

// Workaround https://bugs.llvm.org/show_bug.cgi?id=35202
template <typename P>
struct is_total_ordering : std::false_type {};

template <>
struct is_total_ordering<std::less<>> : std::true_type {};

template <typename T>
struct equality_is_well_defined
    : std::is_integral<typename std::decay<T>::type> {};

template <typename T, typename U, typename P>
using can_use_eqality = std::integral_constant<bool,
  std::is_same<typename std::decay<T>::type,
               typename std::decay<U>::type>::value &&
  equality_is_well_defined<U>::value &&
  is_total_ordering<P>::value>;

template <typename T, typename U, typename P>
typename std::enable_if<!can_use_eqality<T, U, P>::value, bool>::type
not_equal_or_inverse(const T& x, const U& y, P p) { return p(y, x); }

template <typename T, typename U, typename P>
typename std::enable_if<can_use_eqality<T, U, P>::value, bool>::type
not_equal_or_inverse(const T& x, const U& y, P p) { return !(x == y); }

namespace v6 {

template <class I1, class I2, class O, class Comp>
O set_union(I1 f1, I1 l1, I2 f2, I2 l2, O o, Comp comp) {
  if (f1 == l1) goto copySecond;
  if (f2 == l2) goto copyFirst;

  while (true) {
    if (__builtin_expect(comp(*f1, *f2), true)) {
      *o++ = *f1++;
      if (f1 == l1) goto copySecond;
    } else {
      if (not_equal_or_inverse(*f1, *f2, comp)) *o++ = *f2;
      ++f2; if (f2 == l2) goto copyFirst;
    }
  }

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

  notBiased:
    if (__builtin_expect(comp(*f1, *f2), true)) {
      *o++ = *f1++; if (f1 == l1) goto copySecond;
      goto biased;
    } else {
  checkSecond:
      if (comp(*f2, *f1)) *o++ = *f2;
      ++f2; if (f2 == l2) goto copyFirst;
      goto notBiased;
    }
  biased:
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    if (!comp(*f1, *f2)) goto checkSecond;
    *o++ = *f1++; if (f1 == l1) goto copySecond;
    goto biased;

copySecond:
  return std::copy(f2, l2, o);
copyFirst:
  return std::copy(f1, l1, o);
}

}  // namespace v7

#include <algorithm>
#include <exception>
#include <random>
#include <set>
#include <vector>

namespace {

constexpr size_t kProblemSize = 2000;
constexpr size_t kMinSize = 0;
constexpr size_t kStep = 40;
constexpr bool kLastStep = false;

using int_vec = std::vector<int>;

std::pair<int_vec, int_vec> test_input_data(size_t lhs_size, size_t rhs_size) {
  static std::map<std::pair<size_t, size_t>, std::pair<int_vec, int_vec>>
      cached_results;

  auto in_cache = cached_results.find({lhs_size, rhs_size});
  if (in_cache != cached_results.end())
    return in_cache->second;

  auto random_number = [] {
    static std::mt19937 g;
    static std::uniform_int_distribution<> dis(1, int(kProblemSize) * 100);
    return dis(g);
  };

  auto generate_vec = [&](size_t size) {
    std::set<int> res;
    while (res.size() < size)
      res.insert(random_number());
    return int_vec(res.begin(), res.end());
  };

  auto res_and_bool = cached_results.insert(
      {{lhs_size, rhs_size}, {generate_vec(lhs_size), generate_vec(rhs_size)}});
  if (!res_and_bool.second)
    std::terminate();
  return res_and_bool.first->second;
}

template <typename Alg>
void set_union_bench(benchmark::State& state) {
  const size_t lhs_size = static_cast<size_t>(state.range(0));
  const size_t rhs_size = static_cast<size_t>(state.range(1));

  auto input = test_input_data(lhs_size, rhs_size);

  for (auto _ : state) {
    int_vec res(lhs_size + rhs_size);
    Alg{}(input.first.begin(), input.first.end(), input.second.begin(),
          input.second.end(), res.begin());
  }
}

void full_problem_size(benchmark::internal::Benchmark* bench) {
  size_t lhs_size = kMinSize;
  size_t rhs_size = kProblemSize - kMinSize;

  do {
    bench->Args({static_cast<int>(lhs_size), static_cast<int>(rhs_size)});
    lhs_size += kStep;
    rhs_size -= kStep;
  } while (lhs_size <= kProblemSize);
}

void last_step(benchmark::internal::Benchmark* bench) {
  size_t lhs_size = kProblemSize - kStep;
  size_t rhs_size = kStep;
  do {
    bench->Args({static_cast<int>(lhs_size), static_cast<int>(rhs_size)});
    lhs_size += 1;
    rhs_size -= 1;
  } while (lhs_size <= kProblemSize);
}

void set_input_sizes(benchmark::internal::Benchmark* bench) {
  bench->Args({560, 1440});
  return;
  if (kLastStep) {
    last_step(bench);
  } else {
    full_problem_size(bench);
  }
}

struct baseline_alg {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return o;
  }
};

void baseline(benchmark::State& state) {
  set_union_bench<baseline_alg>(state);
}

BENCHMARK(baseline)->Apply(set_input_sizes);

struct linear_set_union {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return v6::set_union(f1, l1, f2, l2, o, std::less<>{});
  }
};

void LinearSetUnion(benchmark::State& state) {
  set_union_bench<linear_set_union>(state);
}

BENCHMARK(LinearSetUnion)->Apply(set_input_sizes);

struct current_set_union {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return v7::set_union(f1, l1, f2, l2, o, std::less<>{});
  }
};

void CurrentSetUnion(benchmark::State& state) {
  set_union_bench<current_set_union>(state);
}

BENCHMARK(CurrentSetUnion)->Apply(set_input_sizes);

}  // namespace

BENCHMARK_MAIN();
