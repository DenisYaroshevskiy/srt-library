clang++ --std=c++14 -O3 -Werror -Wall $1              \
  -I ./                                               \
  -I /space/google_benchmark/benchmark/include/       \
  -I /space/chromium/src/                             \
   /space/google_benchmark/build/src/libbenchmark.a
