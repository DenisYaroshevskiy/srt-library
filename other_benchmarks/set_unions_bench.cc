#include <algorithm>
#include <exception>
#include <random>
#include <set>

#include "set_unions.h"

#include "benchmark/benchmark.h"

#define LAST_STEP

namespace {

constexpr size_t kProblemSize = 2000;
constexpr size_t kMinSize = 0;
constexpr size_t kStep = 40;

#ifdef LAST_STEP
constexpr bool kLastStep = true;
#else
constexpr bool kLastStep = false;
#endif  // LAST_STEP

using int_vec = std::vector<int>;

void full_problem_size(benchmark::internal::Benchmark* bench) {
  size_t lhs_size = kProblemSize - kMinSize;
  size_t rhs_size = kMinSize;

  do {
    bench->Args({static_cast<int>(lhs_size), static_cast<int>(rhs_size)});
    lhs_size -= kStep;
    rhs_size += kStep;
  } while (rhs_size <= kProblemSize);
}

void last_step(benchmark::internal::Benchmark* bench) {
  size_t lhs_size = kProblemSize;
  size_t rhs_size = 0;
  do {
    bench->Args({static_cast<int>(lhs_size), static_cast<int>(rhs_size)});
    --lhs_size;
    ++rhs_size;
  } while (rhs_size <= kStep);
}

}  // namespace

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

void set_input_sizes(benchmark::internal::Benchmark* bench) {
  if (kLastStep) {
    last_step(bench);
  } else {
    full_problem_size(bench);
  }
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

struct baseline_alg {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return o;
  }
};

#ifndef LAST_STEP

struct linear_set_union {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return v7::set_union(f1, l1, f2, l2, o, std::less<>{});
  }
};

#endif  // !LAST_STEP

struct previous_set_union {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return v3::set_union(f1, l1, f2, l2, o, std::less<>{});
  }
};

struct current_set_union {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return v5::set_union(f1, l1, f2, l2, o, std::less<>{});
  }
};

void baseline(benchmark::State& state) {
  set_union_bench<baseline_alg>(state);
}

#ifndef LAST_STEP

void LinearSetUnion(benchmark::State& state) {
  set_union_bench<linear_set_union>(state);
}

#endif  // LAST_STEP

void PreviousSetUnion(benchmark::State& state) {
  set_union_bench<previous_set_union>(state);
}

void CurrentSetUnion(benchmark::State& state) {
  set_union_bench<current_set_union>(state);
}

BENCHMARK(baseline)->Apply(set_input_sizes);

#ifndef LAST_STEP
// BENCHMARK(LinearSetUnion)->Apply(set_input_sizes);
#endif  // LAST_STEP

// BENCHMARK(PreviousSetUnion)->Apply(set_input_sizes);

BENCHMARK(CurrentSetUnion)->Apply(set_input_sizes);

BENCHMARK_MAIN();
