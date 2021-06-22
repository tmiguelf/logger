#include <cstdint>
#include <string_view>

extern const std::string_view test_string;
extern const int32_t test_signed_int;
extern const uint64_t test_unsigned_int;
extern const double test_fp;
extern const char test_char;

void dump_output(std::string_view);

#include <benchmark/benchmark.h>
