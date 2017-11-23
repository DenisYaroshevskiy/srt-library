#include <algorithm>
#include <exception>
#include <random>
#include <set>

#include <boost/container/flat_set.hpp>
#include <folly/sorted_vector_types.h>

#include "benchmark/benchmark.h"

namespace {

constexpr size_t kProblemSize = 2000;
constexpr size_t kMinSize = 0;
constexpr size_t kStep = 40;
constexpr bool kLastStep = true;

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

template <typename Container>
void insert_first_last_bench(benchmark::State& state) {
  const size_t lhs_size = static_cast<size_t>(state.range(0));
  const size_t rhs_size = static_cast<size_t>(state.range(1));

  auto input = test_input_data(lhs_size, rhs_size);
  const Container cached(input.first.begin(), input.first.end());

  for (auto _ : state) {
    Container c(cached);
    c.insert(input.second.begin(), input.second.end());
  }
}

struct baseline_set {
  template <typename I>
  baseline_set(I f, I l)
      : body(f, l){};

  template <typename I>
  void insert(I f, I l) {}

  void reserve(size_t size) { body.reserve(size); }
  size_t size() { return body.size(); }

  int_vec body;
};

void baseline(benchmark::State& state) {
  insert_first_last_bench<baseline_set>(state);
}
BENCHMARK(baseline)->Apply(set_input_sizes);

void Boost(benchmark::State& state) {
  insert_first_last_bench<boost::container::flat_set<int>>(state);
}
BENCHMARK(Boost)->Apply(set_input_sizes);

void Folly(benchmark::State& state) {
  insert_first_last_bench<folly::sorted_vector_set<int>>(state);
}
BENCHMARK(Folly)->Apply(set_input_sizes);

BENCHMARK_MAIN();
