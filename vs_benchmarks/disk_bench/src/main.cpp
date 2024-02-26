//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///
///	\copyright
///		Copyright (c) 2020 Tiago Miguel Oliveira Freire
///
///		Permission is hereby granted, free of charge, to any person obtaining a copy
///		of this software and associated documentation files (the "Software"), to deal
///		in the Software without restriction, including without limitation the rights
///		to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///		copies of the Software, and to permit persons to whom the Software is
///		furnished to do so, subject to the following conditions:
///
///		The above copyright notice and this permission notice shall be included in all
///		copies or substantial portions of the Software.
///
///		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///		IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///		FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///		AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///		LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///		OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///		SOFTWARE.
//======== ======== ======== ======== ======== ======== ======== ========

#include <cstdint>
#include <string_view>

#include <CoreLib/core_console.hpp>
#include <CoreLib/toPrint/toPrint.hpp>
#include <CoreLib/core_time.hpp>
#include <CoreLib/core_thread.hpp>
#include <CoreLib/core_sync.hpp>

#include "disk_logger.hpp"
#include "disk_async_logger.hpp"
#include "disk_spdlog.hpp"
#include "disk_NanoLog.hpp"
#include "disk_g3log.hpp"

using namespace std::literals;

const std::string_view test_string = "The quick brown fox jumps over the lazy dog";
const int32_t test_signed_int = -34;
const uint64_t test_unsigned_int = 12345;
const double test_fp = 5.67;
const char test_char = 'a';

using func_t = void (*)();
using log_func = void (*)(std::string_view, int32_t, uint64_t, double, char );


void ThreadRunner(void* p_test)
{
	log_func func = reinterpret_cast<log_func>(p_test);


	int32_t signed_int = test_signed_int;
	uint64_t unsigned_int = test_unsigned_int;
	double fp = test_fp;
	char t_char = test_char;

	for(uint64_t i = 100000; i--;)
	{
		func(test_string, signed_int, unsigned_int, fp, t_char);
		++signed_int;
		++unsigned_int;
		if(++t_char > 126) t_char = 32;

		++reinterpret_cast<uint64_t&>(fp);
	}
}

void RunTest(func_t p_int, log_func p_call, func_t p_clean, std::string_view p_name)
{
	p_int();
	std::array<core::thread, 5> threads;
	const uint64_t start = core::clock_stamp();

	for(core::thread& t_thread : threads)
	{
		t_thread.create(ThreadRunner, reinterpret_cast<void*>(p_call));
	}

	for(core::thread& t_thread : threads)
	{
		if(t_thread.join() != core::thread::Error::None) exit(1);
	}


	p_clean();

	const uint64_t end = core::clock_stamp();



	core::print<char8_t>(core::cout, p_name, " time: "sv, static_cast<double>(end - start) / 1'000'000'000.0, "s\n"sv);
}


#ifdef _WIN32
int wmain(
	[[maybe_unused]] int argc,
	[[maybe_unused]] wchar_t** argv,
	[[maybe_unused]] wchar_t** envp)
#else
int main(
	[[maybe_unused]] int argc,
	[[maybe_unused]] char** argv,
	[[maybe_unused]] char** envp)
#endif
{
	RunTest(disk_logger::testSetup, disk_logger::log, disk_logger::testClean, "Logger");
	RunTest(disk_async_logger::testSetup, disk_async_logger::log, disk_async_logger::testClean, "Logger async");
	RunTest(disk_spdlog::testSetup, disk_spdlog::log, disk_spdlog::testClean, "spdlog");
	RunTest(disk_NanoLog::testSetup, disk_NanoLog::log, disk_NanoLog::testClean, "NanoLog");
	RunTest(disk_g3log::testSetup, disk_g3log::log, disk_g3log::testClean, "g3log");

	return 0;
}
