#include <random>
#include <set>
#include <unordered_set>
#include <vector>

#include "srt.h"

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

template <typename Contaier>
void range_construction(benchmark::State& state) {
  std::vector<value_type> v = generate_input();
  const Contaier c(v.begin(), v.end());

  while(state.KeepRunning())
    benchmark::DoNotOptimize(Contaier(c));
}

}  // namespace

BENCHMARK_TEMPLATE(range_construction, srt::flat_set<value_type>);
BENCHMARK_TEMPLATE(range_construction, std::unordered_set<value_type>);
BENCHMARK_TEMPLATE(range_construction, std::set<value_type>);

BENCHMARK_MAIN();
