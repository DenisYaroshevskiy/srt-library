#include <random>
#include <set>
#include <unordered_set>
#include <vector>

#include "srt.h"

#include "benchmark/benchmark.h"

namespace {

constexpr size_t kSize = 1000;
constexpr size_t kStep = 15;
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
void find_element(benchmark::State& state) {
  std::vector<value_type> v = generate_input();
  Contaier c(v.begin(), v.end());
  size_t looking_for_idx = 0;

  while(state.KeepRunning()) {
    looking_for_idx += kStep;
    looking_for_idx %= v.size();
    benchmark::DoNotOptimize(c.find(v[looking_for_idx]));
  }
}

}  // namespace

BENCHMARK_TEMPLATE(find_element, std::unordered_set<value_type>);
BENCHMARK_TEMPLATE(find_element, srt::flat_set<value_type>);
BENCHMARK_TEMPLATE(find_element, std::set<value_type>);

BENCHMARK_MAIN();
