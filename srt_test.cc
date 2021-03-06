#include "srt.h"

#include <algorithm>
#include <functional>
#include <list>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "srt_test_templates.h"

namespace {

struct strange_cmp : srt::less {
  explicit strange_cmp(int) {}
};

using int_set = srt::flat_set<int>;
using move_only_set = srt::flat_set<move_only_int>;
using std_int_vec = int_set::underlying_type;
using strange_cmp_set = srt::flat_set<int, strange_cmp>;
using reverse_set = srt::flat_set<int, std::greater<int>>;

struct template_constructor {
  template <typename T>
  template_constructor(const T&) {}

  friend bool operator<(const template_constructor&,
                        const template_constructor&) {
    return false;
  }
};

struct no_default_or_copy {
  explicit no_default_or_copy(int x) : body(x) {}
  no_default_or_copy(no_default_or_copy&&) = default;
  no_default_or_copy& operator=(no_default_or_copy&&) = default;

  friend bool operator<(const no_default_or_copy& x,
                        const no_default_or_copy& y) {
    return x.body < y.body;
  }

  friend bool operator==(const no_default_or_copy& x,
                         const no_default_or_copy& y) {
    return x.body == y.body;
  }

  friend std::ostream& operator<<(std::ostream& out,
                                  const no_default_or_copy& x) {
    out << x.body;
    return out;
  }

  int body;
};

static_assert(!std::is_default_constructible<no_default_or_copy>::value, "");
static_assert(!std::is_copy_constructible<no_default_or_copy>::value, "");
static_assert(!std::is_copy_assignable<no_default_or_copy>::value, "");

template <typename I, typename Op>
void lower_bound_test_run_for_one_input(I f, I l, Op op) {
  for (I looking_for = f; looking_for != l; ++looking_for) {
    REQUIRE(op(f, l, *looking_for) == std::lower_bound(f, l, *looking_for));
  }

  for (int lookingFor :
       {std::numeric_limits<int>::min(), std::numeric_limits<int>::max()}) {
    REQUIRE(op(f, l, lookingFor) == std::lower_bound(f, l, lookingFor));
  }
}

template <typename Container, typename Op>
void lower_bound_run_for_container(const Container& c, Op op) {
  for (auto i = c.begin(); i != c.end(); ++i) {
    lower_bound_test_run_for_one_input(c.begin(), i, op);
  }
}

template <typename Op>
void lower_bound_test(Op op) {
  size_t kSize = 200;
  std::vector<int> inputs(kSize);

  for (int i = 0, j = 0; i < static_cast<int>(kSize);) {
    inputs[i++] = j;
    inputs[i++] = j;
    ++j;
  }

  lower_bound_run_for_container(inputs, op);

  std::list<int> list_inputs(inputs.begin(), inputs.end());

  lower_bound_run_for_container(list_inputs, op);
}

}  // namespace

// algorithms -----------------------------------------------------------------

TEST_CASE("copy_until_sorted", "[algorithms]") {
  constexpr size_t kSize = 10;
  std::vector<int> vec(kSize);
  std::iota(vec.begin(), vec.end(), 1);

  srt::copy_until_sorted(vec.begin(), vec.begin(), vec.begin());

  for (size_t size = kSize; size > 0; --size) {
    std::fill(vec.begin() + size, vec.end(), 0);

    std::vector<int> actual_output;
    srt::copy_until_sorted(vec.begin(), vec.end(),
                           std::back_inserter(actual_output));

    std::vector<int> actual_fwd(size);
    srt::copy_until_sorted(vec.begin(), vec.end(), actual_fwd.begin());

    std::vector<int> expected(size);
    std::iota(expected.begin(), expected.end(), 1);
    REQUIRE(expected == actual_output);
    REQUIRE(expected == actual_fwd);
  }
}

TEST_CASE("int_tmp_buffer", "[temporary_buffer]") {
  srt::temporary_buffer<int> buf(3);
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
  srt::temporary_buffer<std::string> buf(3);
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

struct lower_bound_functor {
  template <typename I, typename V>
  I operator()(I f, I l, const V& v) {
    return srt::lower_bound(f, l, v);
  }
};

TEST_CASE("lower_bound", "[algorithms]") {
  lower_bound_test(lower_bound_functor());
}

struct lower_bound_biased_functor {
  template <typename I, typename V>
  I operator()(I f, I l, const V& v) {
    return srt::lower_bound_biased(f, l, v);
  }
};

TEST_CASE("lower_bound_biased", "[algorithms]") {
  lower_bound_test(lower_bound_biased_functor());
}

TEST_CASE("lower_bound_hinted", "[algorithms]") {
  std::vector<int> vec(100);
  std::iota(vec.begin(), vec.end(), 1);
  for (int x = 0; x <= 101; ++x) {
    auto hint = vec.begin();
    for (auto size = vec.size() + 1; size; --size) {
      REQUIRE(std::lower_bound(vec.begin(), vec.end(), x) ==
              srt::lower_bound_hinted(vec.begin(), hint, vec.end(), x));
      ++hint;
    }
  }
}

TEST_CASE("rotate_buffered", "[algorithms]") {
  using I = std_int_vec::iterator;
  rotate_test([](I f, I m, I l) { return srt::rotate_buffered(f, m, l); });
}

TEST_CASE("inplace_merge_rotating_middles", "[algorithms]") {
  using I = std_int_vec::iterator;
  inplace_merge_test([](I f, I m, I l) {
    return srt::inplace_merge_rotating_middles(f, m, l);
  });
}

TEST_CASE("inplace_merge_rotating_middles_buffered", "[algorithms]") {
  using I = std_int_vec::iterator;
  inplace_merge_test([](I f, I m, I l) {
    return srt::inplace_merge_rotating_middles_buffered(f, m, l);
  });
}

struct set_union_unique_linear_functor {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) const {
    return srt::set_union_unique_linear(f1, l1, f2, l2, o);
  }
};

TEST_CASE("set_union_unique_linear", "[algorithms]") {
  set_union_unique_test(set_union_unique_linear_functor{});
}

struct set_union_unique_biased {
  template <typename I1, typename I2, typename O>
  O operator()(I1 f1, I1 l1, I2 f2, I2 l2, O o) const {
    return srt::set_union_unique_biased(f1, l1, f2, l2, o);
  }
};

TEST_CASE("set_union_unique_biased", "[algorithms]") {
  set_union_unique_test(set_union_unique_biased{});
}

TEST_CASE("resize_with_junk int", "[algorithms]") {
  std_int_vec v;
  int int_sample = 0;
  srt::resize_with_junk(v, int_sample, 3);
  REQUIRE(3 == v.size());
}

TEST_CASE("resize_with_junk weird type", "[algorithms]") {
  std::vector<no_default_or_copy> v;
  no_default_or_copy sample(0);

  srt::resize_with_junk(v, sample, 3);
  REQUIRE(3 == v.size());

  srt::resize_with_junk(v, sample, 1);
  REQUIRE(1 == v.size());

  srt::resize_with_junk(v, sample, 5);
  REQUIRE(5 == v.size());
}

// flat_set ---------------------------------------------------------------

TEST_CASE("flat_set_types", "[flat_cainers, flat_set]") {
  // These are guaranteed to be portable.
  static_assert((std::is_same<int, int_set::key_type>::value), "");
  static_assert((std::is_same<int, int_set::value_type>::value), "");
  static_assert((std::is_same<srt::less, int_set::key_compare>::value), "");
  static_assert((std::is_same<int&, int_set::reference>::value), "");
  static_assert((std::is_same<const int&, int_set::const_reference>::value),
                "");
  static_assert((std::is_same<int*, int_set::pointer>::value), "");
  static_assert((std::is_same<const int*, int_set::const_pointer>::value), "");

  static_assert(sizeof(int_set) == sizeof(int_set::underlying_type), "");
}

TEST_CASE("flat_set_incomplete_type", "[flat_cainers, flat_set]") {
  struct t {
    using dependent = srt::flat_set<t>;
    int data;
    dependent set_with_incomplete_type;
    dependent::iterator it;
    dependent::const_iterator cit;

    // We do not declare operator< because clang complains that it's unused.
  };

  t x;
}

TEST_CASE("flat_set_default_constructor", "[flat_cainers, flat_set]") {
  {
    const int_set c;
    REQUIRE(c.body() == std_int_vec());
  }
  {
    const strange_cmp_set c(strange_cmp(0));
    REQUIRE(c.body() == std_int_vec());
  }
}

TEST_CASE("flat_set_range_constructor", "[flat_cainers, flat_set]") {
  {
    const int_set c(int_set({1, 1, 2, 2, 1, 2, 3, 3, 3}));
    const std_int_vec expected = {1, 2, 3};
    REQUIRE(expected == c.body());
  }
  {
    const int_set c{1, 1, 2, 2, 1, 2, 3, 3, 3};
    const std_int_vec expected = {1, 2, 3};
    REQUIRE(expected == c.body());
  }
  {
    const std_int_vec input{1, 1, 2, 2, 1, 2, 3, 3, 3};
    const int_set c(input.begin(), input.end());
    const std_int_vec expected = {1, 2, 3};
    REQUIRE(expected == c.body());
  }
  {
    std_int_vec input{1, 1, 2, 2, 1, 2, 3, 3, 3};
    const int_set c(std::move(input));
    const std_int_vec expected = {1, 2, 3};
    REQUIRE(expected == c.body());
  }
}

TEST_CASE("flat_set_copy_constructor", "[flat_cainers, flat_set]") {
  const int_set original{1, 2, 3, 4};
  auto copy(original);
  std_int_vec expected{1, 2, 3, 4};

  REQUIRE(copy == original);
  REQUIRE(expected == original.body());
  REQUIRE(expected == copy.body());
}

TEST_CASE("flat_set_move_constructor", "[flat_cainers, flat_set]") {
  int input_range[] = {1, 2, 3, 4};

  move_only_set original(std::begin(input_range), std::end(input_range));
  move_only_set moved = std::move(original);

  REQUIRE(1U == moved.count(1));
  REQUIRE(1U == moved.count(2));
  REQUIRE(1U == moved.count(3));
  REQUIRE(1U == moved.count(4));
}

TEST_CASE("flat_set_copy_assignment", "[flat_cainers, flat_set]") {
  const int_set original{1, 2, 3, 4};
  int_set copy;
  copy = original;

  std_int_vec expected{1, 2, 3, 4};

  REQUIRE(copy == original);
  REQUIRE(expected == original.body());
  REQUIRE(expected == copy.body());
}

TEST_CASE("flat_set_move_assignment", "[flat_cainers, flat_set") {
  int input_range[] = {1, 2, 3, 4};

  move_only_set original(std::begin(input_range), std::end(input_range));
  move_only_set moved;
  moved = std::move(original);

  REQUIRE(1U == moved.count(1));
  REQUIRE(1U == moved.count(2));
  REQUIRE(1U == moved.count(3));
  REQUIRE(1U == moved.count(4));
}

TEST_CASE("flat_set_initializer_list_assignment", "[flat_cainers, flat_set") {
  int_set c = {0};
  c = {1, 2, 3, 4};

  std_int_vec expected = {1, 2, 3, 4};

  REQUIRE(expected == c.body());
}

TEST_CASE("flat_set_reserve", "[flat_cainers, flat_set]") {
  int_set c = {1, 2, 3};
  c.reserve(5);
  REQUIRE(5U <= c.capacity());
}

TEST_CASE("flat_set_capacity", "[flat_cainers, flat_set]") {
  int_set c = {1, 2, 3};
  REQUIRE(c.size() <= c.capacity());
  c.reserve(5);
  REQUIRE(c.size() <= c.capacity());
}

TEST_CASE("flat_set_shrink_to_fit", "[flat_cainers, flat_set]") {
  int_set c = {1, 2, 3};

  auto capacity_before = c.capacity();
  c.shrink_to_fit();
  REQUIRE(c.capacity() <= capacity_before);
}

TEST_CASE("flat_set_clear", "[flat_cainers, flat_set]") {
  int_set c{1, 2, 3, 4, 5, 6, 7, 8};
  c.clear();
  REQUIRE(c.body() == std_int_vec());
}

TEST_CASE("flat_set_size", "[flat_cainers, flat_set]") {
  int_set c;

  REQUIRE(0U == c.size());
  c.insert(2);
  REQUIRE(1U == c.size());
  c.insert(1);
  REQUIRE(2U == c.size());
  c.insert(3);
  REQUIRE(3U == c.size());
  c.erase(c.begin());
  REQUIRE(2U == c.size());
  c.erase(c.begin());
  REQUIRE(1U == c.size());
  c.erase(c.begin());
  REQUIRE(0U == c.size());
}

TEST_CASE("flat_set_empty", "[flat_cainers, flat_set]") {
  int_set c;

  REQUIRE(c.empty());
  c.insert(1);
  REQUIRE(!c.empty());
  c.clear();
  REQUIRE(c.empty());
}

TEST_CASE("flat_set_iterators", "[flat_cainers, flat_set]") {
  int_set c{1, 2, 3, 4, 5, 6, 7, 8};

  auto size = c.size();

  REQUIRE(size == std::distance(c.begin(), c.end()));
  REQUIRE(size == std::distance(c.cbegin(), c.cend()));
  REQUIRE(size == std::distance(c.rbegin(), c.rend()));
  REQUIRE(size == std::distance(c.crbegin(), c.crend()));

  {
    int_set::iterator it = c.begin();
    int_set::const_iterator c_it = c.cbegin();
    REQUIRE(it == c_it);
    for (int j = 1; it != c.end(); ++it, ++c_it, ++j) {
      REQUIRE(j == *it);
      REQUIRE(j == *c_it);
    }
  }
  {
    int_set::reverse_iterator rit = c.rbegin();
    int_set::const_reverse_iterator c_rit = c.crbegin();
    REQUIRE(rit == c_rit);
    for (int j = size; rit != c.rend(); ++rit, ++c_rit, --j) {
      REQUIRE(j == *rit);
      REQUIRE(j == *c_rit);
    }
  }
}

TEST_CASE("flat_set_insert_emplace_v", "[flat_cainers, flat_set]") {
  std::mt19937 g;
  std::uniform_int_distribution<> dis(1, 1000);

  int_set c;
  int_set c_emplace;
  std::set<int> test;

  for (int i = 0; i < 1000; ++i) {
    int v = dis(g);

    auto actual = c.insert(v);
    auto actual_emplace = c_emplace.emplace(v);
    auto expected = test.insert(v);
    REQUIRE(expected.second == actual.second);
    REQUIRE(expected.second == actual_emplace.second);

    auto actual_distance = std::distance(c.begin(), actual.first);
    auto actual_emplace_distance =
        std::distance(c_emplace.begin(), actual_emplace.first);
    auto expected_distance = std::distance(test.begin(), expected.first);
    REQUIRE(expected_distance == actual_distance);
    REQUIRE(expected_distance == actual_emplace_distance);
  }
}

TEST_CASE("flat_set_insert_emplace_hint_v", "[flat_cainers, flat_set]") {
  std::mt19937 g;
  std::uniform_int_distribution<> dis(1, 1000);

  int_set c;
  int_set c_emplace;
  std::set<int> test;

  for (int i = 0; i < 1000; ++i) {
    int v = dis(g);
    int hint_distance = std::uniform_int_distribution<>(0, c.size())(g);

    auto hint = std::next(c.begin(), hint_distance);
    auto hint_emplace = std::next(c_emplace.begin(), hint_distance);

    auto actual = c.insert(hint, v);
    auto actual_emplace = c_emplace.emplace_hint(hint_emplace, v);
    auto expected = test.insert(v).first;
    REQUIRE(test.size() == c.size());
    REQUIRE(test.size() == c_emplace.size());

    auto actual_distance = std::distance(c.begin(), actual);
    auto actual_emplace_distance =
        std::distance(c_emplace.begin(), actual_emplace);
    auto expected_distance = std::distance(test.begin(), expected);
    REQUIRE(expected_distance == actual_distance);
    REQUIRE(expected_distance == actual_emplace_distance);
  }
}

TEST_CASE("flat_set_insert_f_l", "[flat_cainers, flat_set]") {
  std::mt19937 g;
  std::uniform_int_distribution<> dis(1, 1000);
  auto rand_int = [&] { return dis(g); };

  for (size_t c_size = 0; c_size < 100; ++c_size) {
    for (size_t range_size = 0; range_size < 100; ++range_size) {
      std_int_vec already_in(c_size);
      std::generate(already_in.begin(), already_in.end(), rand_int);

      std_int_vec new_elements(range_size);
      std::generate(new_elements.begin(), new_elements.end(), rand_int);

      int_set actual(already_in);
      actual.insert(new_elements.begin(), new_elements.end());

      std_int_vec expected = already_in;
      expected.insert(expected.end(), new_elements.begin(), new_elements.end());
      expected.erase(srt::sort_and_unique(expected.begin(), expected.end()),
                     expected.end());

      REQUIRE(expected == actual.body());
    }
  }
}

TEST_CASE("flat_set_insert_f_l_weird_types", "[flat_cainers, flat_set]") {
  std::list<no_default_or_copy> values;

  values.emplace_back(3);
  values.emplace_back(1);
  values.emplace_back(2);

  srt::flat_set<no_default_or_copy> c;
  c.emplace(4);
  c.insert(std::make_move_iterator(values.begin()),
           std::make_move_iterator(values.end()));

  std::vector<no_default_or_copy> expected;

  expected.emplace_back(1);
  expected.emplace_back(2);
  expected.emplace_back(3);
  expected.emplace_back(4);

  REQUIRE(expected == c.body());
}

TEST_CASE("flat_set_erase_pos", "[flat_cainers, flat_set]") {
  {
    int_set c{1, 2, 3, 4, 5, 6, 7, 8};
    std_int_vec expected;

    int_set::iterator it = c.erase(std::next(c.cbegin(), 3));
    REQUIRE(std::next(c.cbegin(), 3) == it);
    expected = {1, 2, 3, 5, 6, 7, 8};
    REQUIRE(expected == c.body());

    it = c.erase(std::next(c.begin(), 0));
    REQUIRE(c.begin() == it);
    expected = {2, 3, 5, 6, 7, 8};
    REQUIRE(expected == c.body());

    it = c.erase(std::next(c.cbegin(), 5));
    REQUIRE(c.end() == it);
    expected = {2, 3, 5, 6, 7};
    REQUIRE(expected == c.body());

    it = c.erase(std::next(c.cbegin(), 1));
    REQUIRE(std::next(c.begin()) == it);
    expected = {2, 5, 6, 7};
    REQUIRE(expected == c.body());

    it = c.erase(std::next(c.cbegin(), 2));
    REQUIRE(std::next(c.begin(), 2) == it);
    expected = {2, 5, 7};
    REQUIRE(expected == c.body());

    it = c.erase(std::next(c.cbegin(), 2));
    REQUIRE(std::next(c.begin(), 2) == it);
    expected = {2, 5};
    REQUIRE(expected == c.body());

    it = c.erase(std::next(c.cbegin(), 0));
    REQUIRE(std::next(c.begin(), 0) == it);
    expected = {5};
    REQUIRE(expected == c.body());

    it = c.erase(c.begin());
    REQUIRE(c.begin() == it);
    REQUIRE(c.end() == it);
  }
  //  This is LWG #2059.
  //  There is a potential ambiguity between erase with an iterator and erase
  //  with a key, if key has a templated constructor.
  {
    srt::flat_set<template_constructor> c;
    template_constructor v(0);

    auto it = c.find(v);
    if (it != c.end()) c.erase(it);
  }
}

TEST_CASE("flat_set_erase_range", "[flat_cainers, flat_set]") {
  int_set c{1, 2, 3, 4, 5, 6, 7, 8};

  int_set::iterator it =
      c.erase(std::next(c.cbegin(), 5), std::next(c.cbegin(), 5));
  REQUIRE(std::next(c.begin(), 5) == it);
  std_int_vec expected = {1, 2, 3, 4, 5, 6, 7, 8};
  REQUIRE(expected == c.body());

  it = c.erase(std::next(c.cbegin(), 3), std::next(c.cbegin(), 4));
  REQUIRE(std::next(c.begin(), 3) == it);
  expected = {1, 2, 3, 5, 6, 7, 8};
  REQUIRE(expected == c.body());

  it = c.erase(std::next(c.cbegin(), 2), std::next(c.cbegin(), 5));
  REQUIRE(std::next(c.begin(), 2) == it);
  expected = {1, 2, 7, 8};
  REQUIRE(expected == c.body());

  it = c.erase(std::next(c.cbegin(), 0), std::next(c.cbegin(), 2));
  REQUIRE(std::next(c.begin(), 0) == it);
  expected = {7, 8};
  REQUIRE(expected == c.body());

  it = c.erase(c.cbegin(), c.cend());
  REQUIRE(c.begin() == it);
  REQUIRE(c.end() == it);
}

TEST_CASE("flat_set_erase_v", "[flat_cainers, flat_set]") {
  int_set c{1, 2, 3, 4, 5, 6, 7, 8};

  REQUIRE(0U == c.erase(9));
  std_int_vec expected = {1, 2, 3, 4, 5, 6, 7, 8};
  REQUIRE(expected == c.body());

  REQUIRE(1U == c.erase(4));
  expected = {1, 2, 3, 5, 6, 7, 8};
  REQUIRE(expected == c.body());

  REQUIRE(1U == c.erase(1));
  expected = {2, 3, 5, 6, 7, 8};
  REQUIRE(expected == c.body());

  REQUIRE(1U == c.erase(8));
  expected = {2, 3, 5, 6, 7};
  REQUIRE(expected == c.body());

  REQUIRE(1U == c.erase(3));
  expected = {2, 5, 6, 7};
  REQUIRE(expected == c.body());

  REQUIRE(1U == c.erase(6));
  expected = {2, 5, 7};
  REQUIRE(expected == c.body());

  REQUIRE(1U == c.erase(7));
  expected = {2, 5};
  REQUIRE(expected == c.body());

  REQUIRE(1U == c.erase(2));
  expected = {5};
  REQUIRE(expected == c.body());

  REQUIRE(1U == c.erase(5));
  expected = {};
  REQUIRE(expected == c.body());
}

TEST_CASE("flat_set_key_value_compare", "[flat_cainers, flat_set]") {
  reverse_set c{1, 2, 3, 4, 5};

  REQUIRE(std::is_sorted(c.begin(), c.end(), c.key_comp()));
  REQUIRE(std::is_sorted(c.begin(), c.end(), c.value_comp()));

  int new_elements[] = {6, 7, 8, 9, 10};
  std::copy(std::begin(new_elements), std::end(new_elements),
            std::inserter(c, c.end()));

  REQUIRE(std::is_sorted(c.begin(), c.end(), c.key_comp()));
  REQUIRE(std::is_sorted(c.begin(), c.end(), c.value_comp()));
}

TEST_CASE("flat_set_count", "[flat_cainers, flat_set]") {
  const int_set c{5, 6, 7, 8, 9, 10, 11, 12};

  REQUIRE(1U == c.count(5));
  REQUIRE(1U == c.count(6));
  REQUIRE(1U == c.count(7));
  REQUIRE(1U == c.count(8));
  REQUIRE(1U == c.count(9));
  REQUIRE(1U == c.count(10));
  REQUIRE(1U == c.count(11));
  REQUIRE(1U == c.count(12));
  REQUIRE(0U == c.count(4));
}

TEST_CASE("flat_set_find", "[flat_cainers, flat_set]") {
  {
    int_set c{5, 6, 7, 8, 9, 10, 11, 12};

    int_set::iterator it = c.find(5);
    REQUIRE(c.begin() == it);
    REQUIRE(std::next(c.begin()) == c.find(6));
    REQUIRE(std::next(c.begin(), 2) == c.find(7));
    REQUIRE(std::next(c.begin(), 3) == c.find(8));
    REQUIRE(std::next(c.begin(), 4) == c.find(9));
    REQUIRE(std::next(c.begin(), 5) == c.find(10));
    REQUIRE(std::next(c.begin(), 6) == c.find(11));
    REQUIRE(std::next(c.begin(), 7) == c.find(12));
    REQUIRE(std::next(c.begin(), 8) == c.find(4));
  }
  {
    const int_set c({5, 6, 7, 8, 9, 10, 11, 12});

    int_set::const_iterator it = c.find(5);
    REQUIRE(c.begin() == it);
    REQUIRE(std::next(c.begin()) == c.find(6));
    REQUIRE(std::next(c.begin(), 2) == c.find(7));
    REQUIRE(std::next(c.begin(), 3) == c.find(8));
    REQUIRE(std::next(c.begin(), 4) == c.find(9));
    REQUIRE(std::next(c.begin(), 5) == c.find(10));
    REQUIRE(std::next(c.begin(), 6) == c.find(11));
    REQUIRE(std::next(c.begin(), 7) == c.find(12));
    REQUIRE(std::next(c.begin(), 8) == c.find(4));
  }
}

TEST_CASE("flat_set_equal_range", "[flat_cainers, flat_set]") {
  {
    int_set c({5, 7, 9, 11, 13, 15, 17, 19});

    std::pair<int_set::iterator, int_set::iterator> r = c.equal_range(5);
    REQUIRE(std::next(c.begin(), 0) == r.first);
    REQUIRE(std::next(c.begin(), 1) == r.second);
    r = c.equal_range(7);
    REQUIRE(std::next(c.begin(), 1) == r.first);
    REQUIRE(std::next(c.begin(), 2) == r.second);
    r = c.equal_range(9);
    REQUIRE(std::next(c.begin(), 2) == r.first);
    REQUIRE(std::next(c.begin(), 3) == r.second);
    r = c.equal_range(11);
    REQUIRE(std::next(c.begin(), 3) == r.first);
    REQUIRE(std::next(c.begin(), 4) == r.second);
    r = c.equal_range(13);
    REQUIRE(std::next(c.begin(), 4) == r.first);
    REQUIRE(std::next(c.begin(), 5) == r.second);
    r = c.equal_range(15);
    REQUIRE(std::next(c.begin(), 5) == r.first);
    REQUIRE(std::next(c.begin(), 6) == r.second);
    r = c.equal_range(17);
    REQUIRE(std::next(c.begin(), 6) == r.first);
    REQUIRE(std::next(c.begin(), 7) == r.second);
    r = c.equal_range(19);
    REQUIRE(std::next(c.begin(), 7) == r.first);
    REQUIRE(std::next(c.begin(), 8) == r.second);
    r = c.equal_range(4);
    REQUIRE(std::next(c.begin(), 0) == r.first);
    REQUIRE(std::next(c.begin(), 0) == r.second);
    r = c.equal_range(6);
    REQUIRE(std::next(c.begin(), 1) == r.first);
    REQUIRE(std::next(c.begin(), 1) == r.second);
    r = c.equal_range(8);
    REQUIRE(std::next(c.begin(), 2) == r.first);
    REQUIRE(std::next(c.begin(), 2) == r.second);
    r = c.equal_range(10);
    REQUIRE(std::next(c.begin(), 3) == r.first);
    REQUIRE(std::next(c.begin(), 3) == r.second);
    r = c.equal_range(12);
    REQUIRE(std::next(c.begin(), 4) == r.first);
    REQUIRE(std::next(c.begin(), 4) == r.second);
    r = c.equal_range(14);
    REQUIRE(std::next(c.begin(), 5) == r.first);
    REQUIRE(std::next(c.begin(), 5) == r.second);
    r = c.equal_range(16);
    REQUIRE(std::next(c.begin(), 6) == r.first);
    REQUIRE(std::next(c.begin(), 6) == r.second);
    r = c.equal_range(18);
    REQUIRE(std::next(c.begin(), 7) == r.first);
    REQUIRE(std::next(c.begin(), 7) == r.second);
    r = c.equal_range(20);
    REQUIRE(std::next(c.begin(), 8) == r.first);
    REQUIRE(std::next(c.begin(), 8) == r.second);
  }
  {
    const int_set c{5, 7, 9, 11, 13, 15, 17, 19};

    std::pair<int_set::const_iterator, int_set::const_iterator> r =
        c.equal_range(5);
    REQUIRE(std::next(c.begin(), 0) == r.first);
    REQUIRE(std::next(c.begin(), 1) == r.second);
    r = c.equal_range(7);
    REQUIRE(std::next(c.begin(), 1) == r.first);
    REQUIRE(std::next(c.begin(), 2) == r.second);
    r = c.equal_range(9);
    REQUIRE(std::next(c.begin(), 2) == r.first);
    REQUIRE(std::next(c.begin(), 3) == r.second);
    r = c.equal_range(11);
    REQUIRE(std::next(c.begin(), 3) == r.first);
    REQUIRE(std::next(c.begin(), 4) == r.second);
    r = c.equal_range(13);
    REQUIRE(std::next(c.begin(), 4) == r.first);
    REQUIRE(std::next(c.begin(), 5) == r.second);
    r = c.equal_range(15);
    REQUIRE(std::next(c.begin(), 5) == r.first);
    REQUIRE(std::next(c.begin(), 6) == r.second);
    r = c.equal_range(17);
    REQUIRE(std::next(c.begin(), 6) == r.first);
    REQUIRE(std::next(c.begin(), 7) == r.second);
    r = c.equal_range(19);
    REQUIRE(std::next(c.begin(), 7) == r.first);
    REQUIRE(std::next(c.begin(), 8) == r.second);
    r = c.equal_range(4);
    REQUIRE(std::next(c.begin(), 0) == r.first);
    REQUIRE(std::next(c.begin(), 0) == r.second);
    r = c.equal_range(6);
    REQUIRE(std::next(c.begin(), 1) == r.first);
    REQUIRE(std::next(c.begin(), 1) == r.second);
    r = c.equal_range(8);
    REQUIRE(std::next(c.begin(), 2) == r.first);
    REQUIRE(std::next(c.begin(), 2) == r.second);
    r = c.equal_range(10);
    REQUIRE(std::next(c.begin(), 3) == r.first);
    REQUIRE(std::next(c.begin(), 3) == r.second);
    r = c.equal_range(12);
    REQUIRE(std::next(c.begin(), 4) == r.first);
    REQUIRE(std::next(c.begin(), 4) == r.second);
    r = c.equal_range(14);
    REQUIRE(std::next(c.begin(), 5) == r.first);
    REQUIRE(std::next(c.begin(), 5) == r.second);
    r = c.equal_range(16);
    REQUIRE(std::next(c.begin(), 6) == r.first);
    REQUIRE(std::next(c.begin(), 6) == r.second);
    r = c.equal_range(18);
    REQUIRE(std::next(c.begin(), 7) == r.first);
    REQUIRE(std::next(c.begin(), 7) == r.second);
    r = c.equal_range(20);
    REQUIRE(std::next(c.begin(), 8) == r.first);
    REQUIRE(std::next(c.begin(), 8) == r.second);
  }
}

TEST_CASE("flat_set_lower_bound", "[flat_cainers, flat_set]") {
  {
    int_set c{5, 7, 9, 11, 13, 15, 17, 19};

    REQUIRE(c.begin() == c.lower_bound(5));
    REQUIRE(std::next(c.begin()) == c.lower_bound(7));
    REQUIRE(std::next(c.begin(), 2) == c.lower_bound(9));
    REQUIRE(std::next(c.begin(), 3) == c.lower_bound(11));
    REQUIRE(std::next(c.begin(), 4) == c.lower_bound(13));
    REQUIRE(std::next(c.begin(), 5) == c.lower_bound(15));
    REQUIRE(std::next(c.begin(), 6) == c.lower_bound(17));
    REQUIRE(std::next(c.begin(), 7) == c.lower_bound(19));
    REQUIRE(std::next(c.begin(), 0) == c.lower_bound(4));
    REQUIRE(std::next(c.begin(), 1) == c.lower_bound(6));
    REQUIRE(std::next(c.begin(), 2) == c.lower_bound(8));
    REQUIRE(std::next(c.begin(), 3) == c.lower_bound(10));
    REQUIRE(std::next(c.begin(), 4) == c.lower_bound(12));
    REQUIRE(std::next(c.begin(), 5) == c.lower_bound(14));
    REQUIRE(std::next(c.begin(), 6) == c.lower_bound(16));
    REQUIRE(std::next(c.begin(), 7) == c.lower_bound(18));
    REQUIRE(std::next(c.begin(), 8) == c.lower_bound(20));
  }
  {
    const int_set c{5, 7, 9, 11, 13, 15, 17, 19};

    REQUIRE(c.begin() == c.lower_bound(5));
    REQUIRE(std::next(c.begin()) == c.lower_bound(7));
    REQUIRE(std::next(c.begin(), 2) == c.lower_bound(9));
    REQUIRE(std::next(c.begin(), 3) == c.lower_bound(11));
    REQUIRE(std::next(c.begin(), 4) == c.lower_bound(13));
    REQUIRE(std::next(c.begin(), 5) == c.lower_bound(15));
    REQUIRE(std::next(c.begin(), 6) == c.lower_bound(17));
    REQUIRE(std::next(c.begin(), 7) == c.lower_bound(19));
    REQUIRE(std::next(c.begin(), 0) == c.lower_bound(4));
    REQUIRE(std::next(c.begin(), 1) == c.lower_bound(6));
    REQUIRE(std::next(c.begin(), 2) == c.lower_bound(8));
    REQUIRE(std::next(c.begin(), 3) == c.lower_bound(10));
    REQUIRE(std::next(c.begin(), 4) == c.lower_bound(12));
    REQUIRE(std::next(c.begin(), 5) == c.lower_bound(14));
    REQUIRE(std::next(c.begin(), 6) == c.lower_bound(16));
    REQUIRE(std::next(c.begin(), 7) == c.lower_bound(18));
    REQUIRE(std::next(c.begin(), 8) == c.lower_bound(20));
  }
}

TEST_CASE("flat_set_upper_bound", "[flat_cainers, flat_set]") {
  {
    int_set c{5, 7, 9, 11, 13, 15, 17, 19};

    REQUIRE(std::next(c.begin(), 1) == c.upper_bound(5));
    REQUIRE(std::next(c.begin(), 2) == c.upper_bound(7));
    REQUIRE(std::next(c.begin(), 3) == c.upper_bound(9));
    REQUIRE(std::next(c.begin(), 4) == c.upper_bound(11));
    REQUIRE(std::next(c.begin(), 5) == c.upper_bound(13));
    REQUIRE(std::next(c.begin(), 6) == c.upper_bound(15));
    REQUIRE(std::next(c.begin(), 7) == c.upper_bound(17));
    REQUIRE(std::next(c.begin(), 8) == c.upper_bound(19));
    REQUIRE(std::next(c.begin(), 0) == c.upper_bound(4));
    REQUIRE(std::next(c.begin(), 1) == c.upper_bound(6));
    REQUIRE(std::next(c.begin(), 2) == c.upper_bound(8));
    REQUIRE(std::next(c.begin(), 3) == c.upper_bound(10));
    REQUIRE(std::next(c.begin(), 4) == c.upper_bound(12));
    REQUIRE(std::next(c.begin(), 5) == c.upper_bound(14));
    REQUIRE(std::next(c.begin(), 6) == c.upper_bound(16));
    REQUIRE(std::next(c.begin(), 7) == c.upper_bound(18));
    REQUIRE(std::next(c.begin(), 8) == c.upper_bound(20));
  }
  {
    const int_set c{5, 7, 9, 11, 13, 15, 17, 19};

    REQUIRE(std::next(c.begin(), 1) == c.upper_bound(5));
    REQUIRE(std::next(c.begin(), 2) == c.upper_bound(7));
    REQUIRE(std::next(c.begin(), 3) == c.upper_bound(9));
    REQUIRE(std::next(c.begin(), 4) == c.upper_bound(11));
    REQUIRE(std::next(c.begin(), 5) == c.upper_bound(13));
    REQUIRE(std::next(c.begin(), 6) == c.upper_bound(15));
    REQUIRE(std::next(c.begin(), 7) == c.upper_bound(17));
    REQUIRE(std::next(c.begin(), 8) == c.upper_bound(19));
    REQUIRE(std::next(c.begin(), 0) == c.upper_bound(4));
    REQUIRE(std::next(c.begin(), 1) == c.upper_bound(6));
    REQUIRE(std::next(c.begin(), 2) == c.upper_bound(8));
    REQUIRE(std::next(c.begin(), 3) == c.upper_bound(10));
    REQUIRE(std::next(c.begin(), 4) == c.upper_bound(12));
    REQUIRE(std::next(c.begin(), 5) == c.upper_bound(14));
    REQUIRE(std::next(c.begin(), 6) == c.upper_bound(16));
    REQUIRE(std::next(c.begin(), 7) == c.upper_bound(18));
    REQUIRE(std::next(c.begin(), 8) == c.upper_bound(20));
  }
}

TEST_CASE("flat_set_swap", "[flat_cainers, flat_set]") {
  int_set x = {1, 2, 3};
  int_set y = {4};

  swap(x, y);
  std_int_vec expected = {4};
  REQUIRE(x.body() == expected);
  expected = {1, 2, 3};
  REQUIRE(y.body() == expected);

  y.swap(x);
  expected = {1, 2, 3};
  REQUIRE(x.body() == expected);
  expected = {4};
  REQUIRE(y.body() == expected);
}

TEST_CASE("flat_set_ordering", "[flat_cainers, flat_set]") {
  // Provided comparator does not participate in ordering.
  reverse_set biggest = {3};
  reverse_set smallest = {1};
  reverse_set middle = {1, 2};

  REQUIRE(biggest == biggest);
  REQUIRE(biggest > smallest);
  REQUIRE(smallest < middle);
  REQUIRE(smallest <= middle);
  REQUIRE(middle <= middle);
  REQUIRE(biggest > middle);
  REQUIRE(biggest >= middle);
  REQUIRE(biggest >= biggest);
}

TEST_CASE("flat_set_erase_if", "[flat_cainers, flat_set]") {
  int_set x;
  erase_if(x, [](int) { return false; });
  REQUIRE(x.empty());

  x = {1, 2, 3};
  erase_if(x, [](int e) { return !(e & 1); });
  std_int_vec expected = {1, 3};
  REQUIRE(expected == x.body());

  x = {1, 2, 3, 4};
  erase_if(x, [](int e) { return e & 1; });
  expected = {2, 4};
  REQUIRE(expected == x.body());
}

TEST_CASE("flat_set greater", "[flat_cainers, flat_set]") {
  srt::flat_set<int, std::greater<int>> x{1, 2, 3};

  std_int_vec expected = {3, 2, 1};
  REQUIRE(expected == x.body());

  x.insert({4, 5, 6, 7, 8});

  expected = {8, 7, 6, 5, 4, 3, 2, 1};
  REQUIRE(expected == x.body());
}