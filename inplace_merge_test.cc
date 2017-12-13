#include "inplace_merges.h"

#include <algorithm>
#include <functional>
#include <random>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "srt_test_templates.h"

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
