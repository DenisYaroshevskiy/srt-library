#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

#include "third_party/catch.h"

struct move_only_int {
  int body = 0;

  move_only_int() = default;

  move_only_int(int body) : body(body) {}
  move_only_int(const move_only_int&) = delete;
  move_only_int& operator=(const move_only_int&) = delete;

  move_only_int(move_only_int&& x) noexcept {
    body = x.body;
    x.body = 0;
  }

  move_only_int& operator=(move_only_int&& x) noexcept {
    body = x.body;
    x.body = 0;
    return *this;
  }

  move_only_int clone() const { return move_only_int(body); }

  friend bool operator==(const move_only_int& x, const move_only_int& y) {
    return x.body == y.body;
  }

  friend bool operator<(const move_only_int& x, const move_only_int& y) {
    return x.body < y.body;
  }

  friend std::ostream& operator<<(std::ostream& out, const move_only_int& x) {
    return out << x.body;
  }
};

template <typename Alg>
void set_union_random_test(Alg alg) {
  auto random_number = [] {
    static std::mt19937 g;
    static std::uniform_int_distribution<> dis(1, 10000);
    return dis(g);
  };

  for (size_t lhs_size = 0; lhs_size < 100; ++lhs_size) {
    std::vector<int> lhs(lhs_size);
    std::generate(lhs.begin(), lhs.end(), random_number);
    std::sort(lhs.begin(), lhs.end());
    lhs.erase(std::unique(lhs.begin(), lhs.end()), lhs.end());
    for (size_t rhs_size = 0; rhs_size < 100; ++rhs_size) {
      std::vector<int> rhs(rhs_size);
      std::generate(rhs.begin(), rhs.end(), random_number);
      std::sort(rhs.begin(), rhs.end());
      rhs.erase(std::unique(rhs.begin(), rhs.end()), rhs.end());

      std::vector<int> expected = lhs;
      expected.insert(expected.end(), rhs.begin(), rhs.end());
      std::sort(expected.begin(), expected.end());
      expected.erase(std::unique(expected.begin(), expected.end()),
                     expected.end());

      std::vector<int> actual(lhs.size() + rhs.size());
      actual.erase(
          alg(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), actual.begin()),
          actual.end());

      REQUIRE(expected == actual);
    }
  }
}

template <typename Alg>
void std_set_bug(Alg alg) {
  std::vector<move_only_int> in1, in2;
  in1.emplace_back(2);
  in2.emplace_back(2);

  std::vector<move_only_int> backup1(in1.size());
  std::transform(in1.begin(), in1.end(), backup1.begin(),
                 [](const move_only_int& x) { return x.clone(); });

  std::vector<move_only_int> backup2(in1.size());
  std::transform(in2.begin(), in2.end(), backup2.begin(),
                 [](const move_only_int& x) { return x.clone(); });

  std::vector<move_only_int> expected;
  expected.emplace_back(2);

  std::vector<move_only_int> actual(backup1.size() + backup2.size());
  actual.erase(alg(std::make_move_iterator(backup1.begin()),
                   std::make_move_iterator(backup1.end()),
                   std::make_move_iterator(backup2.begin()),
                   std::make_move_iterator(backup2.end()), actual.begin()),
               actual.end());

  CHECK(expected == actual);
}

template <typename Alg>
void set_union_test(Alg alg) {
  set_union_random_test(alg);
  std_set_bug(alg);
}

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
