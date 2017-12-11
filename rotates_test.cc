#include "rotates.h"

#include <numeric>
#include <string>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "third_party/catch.h"

namespace {

template <typename Alg>
void rotate_test(Alg alg) {
  constexpr size_t kVecSize = 100u;
  std::vector<int> vec(kVecSize);
  std::iota(vec.begin(), vec.end(), 0);
  for (size_t middle = 0; middle < kVecSize; ++middle) {
    auto actual = vec;
    auto actual_point =
        alg(actual.begin(), actual.begin() + middle, actual.end());

    auto expected = vec;
    auto expected_point = std::rotate(
        expected.begin(), expected.begin() + middle, expected.end());

    REQUIRE(expected_point - expected.begin() == actual_point - actual.begin());
    REQUIRE(expected == actual);
  }
}

}  // namespace

TEST_CASE("int_tmp_buffer", "[temporary_buffer]") {
  detail::temporary_buffer<int> buf(3);
  using vec = std::vector<int>;
  vec all_ints{1, 2, 3, 4};

  vec::iterator l;
  int* buf_f;
  int* buf_l;

  std::tie(l, buf_f, buf_l) = buf.copy(all_ints.begin(), all_ints.end());

  REQUIRE(l == all_ints.end() - 1);

  vec in_buffer(buf_f, buf_l);
  REQUIRE(in_buffer == vec({1, 2, 3}));
}

TEST_CASE("string_tmp_buffer", "[temporary_buffer]") {
  detail::temporary_buffer<std::string> buf(3);
  using vec = std::vector<std::string>;
  vec all_ints{"1", "2", "3", "4"};

  vec::iterator l;
  std::string* buf_f;
  std::string* buf_l;

  std::tie(l, buf_f, buf_l) = buf.copy(all_ints.begin(), all_ints.end());

  REQUIRE(l == all_ints.end() - 1);

  vec in_buffer(buf_f, buf_l);
  REQUIRE(in_buffer == vec({"1", "2", "3"}));
}

TEST_CASE("v1_rotate", "[rotate]") {
  rotate_test([](auto f, auto m, auto l) { return v1::rotate(f, m, l); });
}
