#include "inplace_merges.h"

#include <algorithm>
#include <functional>
#include <random>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "third_party/catch.h"

template <typename Alg>
void inplace_merge_test(Alg alg) {
  auto random_number = [] {
    static std::mt19937 g;
    static std::uniform_int_distribution<> dis(1, 10000);
    return dis(g);
  };

  for (size_t lhs_size = 0; lhs_size < 100; ++lhs_size) {
    std::vector<int> lhs(lhs_size);
    std::generate(lhs.begin(), lhs.end(), random_number);
    std::sort(lhs.begin(), lhs.end());
    for (size_t rhs_size = 0; rhs_size < 100; ++rhs_size) {
      std::vector<int> rhs(rhs_size);
      std::generate(rhs.begin(), rhs.end(), random_number);
      std::sort(rhs.begin(), rhs.end());

      auto actual = lhs;
      actual.insert(actual.end(), rhs.begin(), rhs.end());
      alg(actual.begin(), actual.begin() + lhs_size, actual.end());

      std::vector<int> expected = lhs;
      expected.insert(expected.end(), rhs.begin(), rhs.end());
      std::sort(expected.begin(), expected.end());

      REQUIRE(expected == actual);
    }
  }
}

TEST_CASE("v1_implace_merge_rotating_middles", "[inplace_merges]") {
  inplace_merge_test([](auto f, auto m, auto l) {
    v1::inplace_merge_rotating_middles(f, m, l, std::less<>{});
  });
}

TEST_CASE("v2_implace_merge_rotating_middles", "[inplace_merges]") {
  inplace_merge_test([](auto f, auto m, auto l) {
    v2::inplace_merge_rotating_middles(f, m, l, std::less<>{});
  });
}
