#include <algorithm>
#include <numeric>
#include <random>
#include <set>
#include <vector>

#include "srt.h"

#include "benchmark/benchmark.h"

using value_type = int;

constexpr size_t kProblemSize = 2000;
constexpr size_t kInsertingElementsSize = 20;

value_type random_number() {
  static std::mt19937 g;
  static std::uniform_int_distribution<> dis(1, int(kProblemSize) * 100);
  return dis(g);
}

auto generate_problem_set() {
  std::set<value_type> res;
  while (res.size() < kProblemSize)
    res.insert(random_number());
  return std::vector<value_type>(res.begin(), res.end());
}

auto generate_elements_for_insertion() {
  std::vector<value_type> vec(kInsertingElementsSize);
  std::generate(vec.begin(), vec.end(), random_number);
  return vec;
}

void inserting_by_one(benchmark::State& state) {
  auto problem_set = generate_problem_set();
  auto elements = generate_elements_for_insertion();
  for (auto _ : state) {
    auto copy = problem_set;
    for (auto e : elements) {
      auto pos = std::lower_bound(copy.begin(), copy.end(), e);
      if (pos == copy.end() || *pos != e)
        copy.insert(pos, e);
    }
  }
}

void unique(benchmark::State& state) {
  auto problem_set = generate_problem_set();
  auto elements = generate_elements_for_insertion();
  problem_set.insert(problem_set.end(), elements.begin(), elements.end());
  std::sort(problem_set.begin(), problem_set.end());
  for (auto _ : state) {
    auto copy = problem_set;
    copy.erase(std::unique(copy.begin(), copy.end()), copy.end());
  }
}

template <typename Alg>
void inplace_merge(benchmark::State& state) {
  auto problem_set = generate_problem_set();
  auto elements = generate_elements_for_insertion();
  problem_set.insert(problem_set.end(), elements.begin(), elements.end());
  std::sort(problem_set.begin() + kProblemSize, problem_set.end());
  for (auto _ : state) {
    auto copy = problem_set;
    Alg()(copy.begin(), copy.begin() + kProblemSize, copy.end());
  }
}

struct set_union_biased {
  template <typename I>
  void operator()(I f, I m, I l) {
    std::vector<srt::ValueType<I>> buf(m, l);
    using r_it = std::reverse_iterator<I>;
    srt::set_union_biased(r_it(m), r_it(f), r_it(buf.rbegin()),
                          r_it(buf.rend()), r_it(l), std::greater<>{});
  }
};

struct naive_rotating_middles {
  template <typename I>
  void operator()(I f, I m, I l) {
    srt::inplace_merge_rotating_middles(f, m, l);
  }
};

struct naive_rotating_middles_bufferd {
  template <typename I>
  void operator()(I f, I m, I l) {
    srt::inplace_merge_rotating_middles_buffered(f, m, l);
  }
};

struct std_inplace_merge {
  template <typename I>
  void operator()(I f, I m, I l) {
    std::inplace_merge(f, m, l);
  }
};

BENCHMARK(inserting_by_one);
BENCHMARK(unique);
BENCHMARK_TEMPLATE(inplace_merge, set_union_biased);
BENCHMARK_TEMPLATE(inplace_merge, naive_rotating_middles);
BENCHMARK_TEMPLATE(inplace_merge, naive_rotating_middles_bufferd);
BENCHMARK_TEMPLATE(inplace_merge, std_inplace_merge);

BENCHMARK_MAIN();
