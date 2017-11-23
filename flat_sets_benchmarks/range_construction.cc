#include <random>
#include <vector>

#include "base/containers/flat_set.h"
#include "srt.h"
#include <boost/container/flat_set.hpp>
#include <folly/sorted_vector_types.h>

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

  while(state.KeepRunning())
    benchmark::DoNotOptimize(Contaier(v.begin(), v.end()));
}

}  // namespace

using OurSoulution = srt::flat_set<value_type>;
using Folly = folly::sorted_vector_set<value_type>;
using Chromium = base::flat_set<value_type>;
using Boost = boost::container::flat_set<value_type>;

BENCHMARK_TEMPLATE(range_construction, OurSoulution);
BENCHMARK_TEMPLATE(range_construction, Folly);
BENCHMARK_TEMPLATE(range_construction, Chromium);
BENCHMARK_TEMPLATE(range_construction, Boost);

BENCHMARK_MAIN();
