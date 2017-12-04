#include <algorithm>
#include <exception>
#include <iostream>
#include <random>
#include <set>
#include <string>

#include <boost/container/flat_set.hpp>
#include <folly/sorted_vector_types.h>
#include "srt.h"

#include "benchmark/benchmark.h"

namespace {

constexpr size_t kProblemSize = 2000;
constexpr size_t kMinSize = 0;
constexpr size_t kStep = 40;

constexpr size_t kMinStringSize = 5;
constexpr size_t kMaxStringSize = 50;

using vec = std::vector<std::string>;

}  // namespace

const std::string& syms() {
  static std::string res = [] {
    std::string res;

    for (char c = 'a'; c != ('z' + 1); ++c)
      res.push_back(c);

    for (char c = 'A'; c != ('Z' + 1); ++c)
      res.push_back(c);

    for (char c : "!@#$%^&*(){}[]~.,:/")
      res.push_back(c);

    return res;
  }();

  return res;
}

std::string random_string() {
  static std::mt19937 g;
  static std::uniform_int_distribution<size_t> sym_dis(0, syms().size() - 1);
  static std::uniform_int_distribution<size_t> size_dis(kMinStringSize,
                                                        kMaxStringSize);

  std::string res(size_dis(g), 'a');
  std::generate(res.begin(), res.end(),
                [&]() mutable { return syms()[sym_dis(g)]; });
  return res;
}

std::pair<vec, vec> input_data(size_t lhs_size, size_t rhs_size) {
  static std::map<std::pair<size_t, size_t>, std::pair<vec, vec>> cache;

  auto in_cache = cache.find({lhs_size, rhs_size});
  if (in_cache != cache.end())
    return in_cache->second;

  auto generate_unique_sorted_vec = [&](size_t size) {
    std::set<std::string> res;
    while (res.size() < size)
      res.insert(random_string());
    return vec(res.begin(), res.end());
  };

  auto generate_vec = [&](size_t size) {
    vec res(size);
    std::generate(res.begin(), res.end(), random_string);
    return res;
  };

  std::pair<vec, vec> value{generate_unique_sorted_vec(lhs_size),
                            generate_vec(rhs_size)};
  auto& res = cache[{lhs_size, rhs_size}];
  res = std::move(value);
  return res;
}

void full_problem_size(benchmark::internal::Benchmark* bench) {
  size_t lhs_size = kProblemSize - kMinSize;
  size_t rhs_size = kMinSize;

  do {
    bench->Args({static_cast<int>(lhs_size), static_cast<int>(rhs_size)});
    lhs_size -= kStep;
    rhs_size += kStep;
  } while (rhs_size <= kProblemSize);
}

template <typename Container>
void string_insert_first_last_bench(benchmark::State& state) {
  const size_t lhs_size = static_cast<size_t>(state.range(0));
  const size_t rhs_size = static_cast<size_t>(state.range(1));

  auto input = input_data(lhs_size, rhs_size);
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

  vec body;
};

void baseline(benchmark::State& state) {
  string_insert_first_last_bench<baseline_set>(state);
}
BENCHMARK(baseline)->Apply(full_problem_size);

void Srt(benchmark::State& state) {
  string_insert_first_last_bench<srt::flat_set<std::string>>(state);
}
BENCHMARK(Srt)->Apply(full_problem_size);

void Folly(benchmark::State& state) {
  string_insert_first_last_bench<folly::sorted_vector_set<std::string>>(state);
}
BENCHMARK(Folly)->Apply(full_problem_size);

void Boost(benchmark::State& state) {
  string_insert_first_last_bench<boost::container::flat_set<std::string>>(
      state);
}
BENCHMARK(Boost)->Apply(full_problem_size);

BENCHMARK_MAIN();
