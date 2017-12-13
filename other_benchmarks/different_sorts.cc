#include <algorithm>
#include <random>
#include <vector>

#include "benchmark/benchmark.h"

constexpr size_t kSize = 1000;
using value_type = int;

template <typename I>
using ValueType = typename std::iterator_traits<I>::value_type;

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

struct baseline {
  template <typename I>
  void operator()(I f, I l) {
    std::vector<ValueType<I>> copy{f, l};
  }
};

struct std_sort {
  template <typename I>
  void operator()(I f, I l) {
    std::vector<ValueType<I>> copy{f, l};
    std::sort(copy.begin(), copy.end());
  }
};

struct std_stable_sort {
  template <typename I>
  void operator()(I f, I l) {
    std::vector<ValueType<I>> copy{f, l};
    std::stable_sort(copy.begin(), copy.end());
  }
};

struct insertion_binary {
  template <typename I>
  void operator()(I f, I l) {
    std::vector<ValueType<I>> copy;
    copy.reserve(l - f);
    while (f != l) {
      copy.insert(std::upper_bound(copy.begin(), copy.end(), *f), *f);
      ++f;
    }
  }
};

template <typename Alg>
void test_sorting_algorithm(benchmark::State& state) {
  auto input = generate_input();
  for (auto _ : state) {
    Alg{}(input.begin(), input.end());
  }
}

BENCHMARK_TEMPLATE(test_sorting_algorithm, baseline);
BENCHMARK_TEMPLATE(test_sorting_algorithm, std_sort);
BENCHMARK_TEMPLATE(test_sorting_algorithm, std_stable_sort);
BENCHMARK_TEMPLATE(test_sorting_algorithm, insertion_binary);

BENCHMARK_MAIN();
