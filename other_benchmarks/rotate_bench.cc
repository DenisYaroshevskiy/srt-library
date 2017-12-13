#include <algorithm>
#include <numeric>

#include "rotates.h"
#include "benchmark/benchmark.h"

using value_type = int;

constexpr size_t kProblemSize = 100;
constexpr size_t kStep = 40;

auto generate_input() {
  std::vector<value_type> vec(kProblemSize);
  std::iota(vec.begin(), vec.end(), 0);
  return vec;
}

template <typename Alg>
void rotate_bench(benchmark::State& state) {
  const size_t middle_distance = static_cast<size_t>(state.range(1));
  auto input = generate_input();

  for (auto _ : state) {
    benchmark::DoNotOptimize(Alg{}(
        input.begin(), std::next(input.begin(), middle_distance), input.end()));
  }
}

void set_input_sizes(benchmark::internal::Benchmark* bench) {
  for (int middle = 0; middle != kProblemSize; middle += kStep)
    bench->Args({0, middle});
}

struct baseline_alg {
  template <typename I>
  I operator()(I f, I m, I l) {
    return f;
  }
};

struct previous_rotate {
  template <typename I>
  I operator()(I f, I m, I l) {
    return std::rotate(f, m, l);
  }
};

struct current_rotate {
  template <typename I>
  I operator()(I f, I m, I l) {
    return v1::rotate(f, m, l);
  }
};

void baseline(benchmark::State& state) {
  rotate_bench<baseline_alg>(state);
}

void PreviousRotate(benchmark::State& state) {
  rotate_bench<previous_rotate>(state);
}

void CurrentRotate(benchmark::State& state) {
  rotate_bench<current_rotate>(state);
}

BENCHMARK(baseline)->Apply(set_input_sizes);
BENCHMARK(PreviousRotate)->Apply(set_input_sizes);
BENCHMARK(CurrentRotate)->Apply(set_input_sizes);

BENCHMARK_MAIN();
