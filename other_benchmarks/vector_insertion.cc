#include <algorithm>
#include <random>
#include <vector>

#include "benchmark/benchmark.h"

namespace {

constexpr size_t kSize = 1000;
using value_type = int;

std::vector<value_type> generate_input() {
  static auto res = [] {
    std::mt19937 g;
    std::uniform_int_distribution<> dis(1, 10000);

    std::vector<value_type> v(kSize);
    std::generate(v.begin(), v.end(), [&] { return dis(g); });
    return v;
  }();

  return res;
}

void InsertFirstLast(benchmark::State& state) {
  const auto input = generate_input();
  while(state.KeepRunning()) {
    std::vector<value_type> v;
    v.insert(v.end(), input.begin(), input.end());
    benchmark::DoNotOptimize(v);
  }
}

void BackInserterWithReserve(benchmark::State& state) {
  const auto input = generate_input();
  while(state.KeepRunning()) {
    std::vector<value_type> v;
    v.reserve(input.size());
    std::copy(input.begin(), input.end(), std::back_inserter(v));
    benchmark::DoNotOptimize(v);
  }
}

void BackInserterNoReserve(benchmark::State& state) {
  const auto input = generate_input();
  while(state.KeepRunning()) {
    std::vector<value_type> v;
    std::copy(input.begin(), input.end(), std::back_inserter(v));
    benchmark::DoNotOptimize(v);
  }
}

}  // namespace

BENCHMARK(InsertFirstLast);
BENCHMARK(BackInserterWithReserve);
BENCHMARK(BackInserterNoReserve);

BENCHMARK_MAIN();
