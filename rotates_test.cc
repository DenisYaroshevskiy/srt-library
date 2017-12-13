#include "rotates.h"

#define CATCH_CONFIG_MAIN
#include "srt_test_templates.h"

TEST_CASE("v1_rotate", "[rotate]") {
  rotate_test([](auto f, auto m, auto l) { return v1::rotate(f, m, l); });
}
