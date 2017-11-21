#include <algorithm>
#include <random>
#include <set>
#include <unordered_set>
#include <vector>

#include "srt.h"

#include "benchmark/benchmark.h"

namespace {

constexpr size_t kSize = 30'000;
using value_type = int;

std::vector<value_type> generate_input() {
  static auto res = [] {
    std::mt19937 g;
    std::uniform_int_distribution<> dis(1, 1000000);

    std::vector<value_type> v(kSize);
    std::generate(v.begin(), v.end(), [&] { return dis(g); });
    return v;
  }();

  return res;
}

template <typename Contaier>
void construction_by_one(benchmark::State& state) {
  std::vector<value_type> v = generate_input();

  while(state.KeepRunning()) {
    Contaier c;
    std::copy(v.begin(), v.end(), std::inserter(c, c.end()));
    benchmark::DoNotOptimize(c);
  }
}

}  // namespace

BENCHMARK_TEMPLATE(construction_by_one, std::unordered_set<value_type>);
BENCHMARK_TEMPLATE(construction_by_one, std::set<value_type>);
BENCHMARK_TEMPLATE(construction_by_one, srt::flat_set<value_type>);

BENCHMARK_MAIN();
