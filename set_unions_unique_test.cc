#include <algorithm>
#include <iostream>
#include <iterator>
#include <random>
#include <vector>

#include "set_unions_unique.h"

#define CATCH_CONFIG_MAIN
#include "srt_test_templates.h"

namespace {

using int_it = std::vector<int>::iterator;

}  // namespace

#if 0
TEST_CASE("v1_set_union_unique", "[set_union_uniques]") {
  set_union_unique_test([](auto f1, auto l1, auto f2, auto l2, auto o) {
    return v1::set_union_unique(f1, l1, f2, l2, o, std::less<>{});
  });
}
#endif

TEST_CASE("v2_set_union_unique", "[set_union_uniques]") {
  set_union_unique_test([](auto f1, auto l1, auto f2, auto l2, auto o) {
    return v2::set_union_unique(f1, l1, f2, l2, o, std::less<>{});
  });
}

TEST_CASE("v3_set_union_unique", "[set_union_uniques]") {
  set_union_unique_test([](auto f1, auto l1, auto f2, auto l2, auto o) {
    return v3::set_union_unique(f1, l1, f2, l2, o, std::less<>{});
  });
}

TEST_CASE("v4_set_union_unique", "[set_union_uniques]") {
  set_union_unique_test([](auto f1, auto l1, auto f2, auto l2, auto o) {
    return v4::set_union_unique(f1, l1, f2, l2, o, std::less<>{});
  });
}

TEST_CASE("v5_set_union_unique", "[set_union_uniques]") {
  set_union_unique_test([](auto f1, auto l1, auto f2, auto l2, auto o) {
    return v5::set_union_unique(f1, l1, f2, l2, o, std::less<>{});
  });
}

TEST_CASE("v6_set_union_unique", "[set_union_uniques]") {
  set_union_unique_test([](auto f1, auto l1, auto f2, auto l2, auto o) {
    return v6::set_union_unique(f1, l1, f2, l2, o, std::less<>{});
  });
}

TEST_CASE("v7_set_union_unique", "[set_union_uniques]") {
  set_union_unique_test([](auto f1, auto l1, auto f2, auto l2, auto o) {
    return v7::set_union_unique(f1, l1, f2, l2, o, std::less<>{});
  });
}

TEST_CASE("v8_set_union_unique", "[set_union_uniques]") {
  set_union_unique_test([](auto f1, auto l1, auto f2, auto l2, auto o) {
    return v8::set_union_unique(f1, l1, f2, l2, o, std::less<>{});
  });
}

TEST_CASE("v9_set_union_unique", "[set_union_uniques]") {
  set_union_unique_test([](auto f1, auto l1, auto f2, auto l2, auto o) {
    return v9::set_union_unique(f1, l1, f2, l2, o, std::less<>{});
  });
}

TEST_CASE("v10_set_union_unique", "[set_union_uniques]") {
  set_union_unique_test([](auto f1, auto l1, auto f2, auto l2, auto o) {
    return v10::set_union_unique(f1, l1, f2, l2, o, std::less<>{});
  });
}
