#include "common.hpp"


const std::string_view test_string = "The quick brown fox jumps over the lazy fox";
const int32_t test_signed_int = -34;
const uint64_t test_unsigned_int = 12345;
const double test_fp = -5.67;
const char test_char = 'a';

static const volatile char* g_dump;
static volatile uintptr_t g_dump2;

void dump_output(std::string_view p_data)
{
	g_dump = p_data.data();
	g_dump2 = p_data.size();
}
