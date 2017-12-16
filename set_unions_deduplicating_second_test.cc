#include "set_unions_deduplicating_second.h"

#define CATCH_CONFIG_MAIN
#include "srt_test_templates.h"

TEST_CASE("v1_set_union_deduplicating_second",
          "[set_unions_deduplicating_second]") {
  set_union_deduplicating_second_test([](auto f1, auto l1, auto f2, auto l2,
                                         auto o) {
    return v1::set_union_deduplicating_second(f1, l1, f2, l2, o, std::less<>{});
  });
}
