#include <algorithm>
#include <vector>
#include <random>
#include <utility>

#include "set_unions_unique.h"

#include "benchmark/benchmark.h"

namespace {

constexpr size_t kProblemSize = 1000u;
constexpr size_t kEveryNthInSecond = 6u;

using int_vec = std::vector<int>;

std::pair<int_vec, int_vec> input() {
  auto random_number = [] {
    static std::mt19937 g;
    static std::uniform_int_distribution<> dis(1, int(kProblemSize) * 100);
    return dis(g);
  };

  int_vec total(kProblemSize);

  std::generate(total.begin(), total.end(), random_number);
  std::sort(total.begin(), total.end());

  int_vec lhs, rhs;

  for (size_t i = 0; i < kProblemSize; ++i) {
    if (i % kEveryNthInSecond)
      lhs.push_back(total[i]);
    else
      rhs.push_back(total[i]);
  }

  return {lhs, rhs};
}

}  // namespace

template <typename Alg>
void set_union_bench(benchmark::State& state) {
  int_vec lhs, rhs;
  std::tie(lhs, rhs) = input();

  for (auto _ : state) {
    int_vec res(lhs.size() + rhs.size());
    Alg{}(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), res.begin());
  }
}

struct linear {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return v7::set_union_unique(f1, l1, f2, l2, o, std::less<>{});
  }
};

struct biased {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) {
    return v10::set_union_unique(f1, l1, f2, l2, o, std::less<>{});
  }
};

void Linear(benchmark::State& state) {
  set_union_bench<linear>(state);
}
BENCHMARK(Linear);

void Biased(benchmark::State& state) {
  set_union_bench<biased>(state);
}
BENCHMARK(Biased);

BENCHMARK_MAIN();
