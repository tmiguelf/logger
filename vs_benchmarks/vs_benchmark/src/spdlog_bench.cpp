//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///
///	\copyright
///		Copyright (c) Tiago Miguel Oliveira Freire
///
///		Permission is hereby granted, free of charge, to any person obtaining a copy
///		of this software and associated documentation files (the "Software"),
///		to copy, modify, publish, and/or distribute copies of the Software,
///		and to permit persons to whom the Software is furnished to do so,
///		subject to the following conditions:
///
///		The copyright notice and this permission notice shall be included in all
///		copies or substantial portions of the Software.
///		The copyrighted work, or derived works, shall not be used to train
///		Artificial Intelligence models of any sort; or otherwise be used in a
///		transformative way that could obfuscate the source of the copyright.
///
///		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///		IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///		FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///		AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///		LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///		OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///		SOFTWARE.
//======== ======== ======== ======== ======== ======== ======== ========

#include "common.hpp"

#include <spdlog/spdlog.h>
#include <CoreLib/core_time.hpp>
class dumpSink_spdlog: public spdlog::sinks::sink
{
public:
	void log(spdlog::details::log_msg const& msg) override
	{
		//forced collection of timestamp for test fairness
		[[maybe_unused]] auto volatile temp = std::chrono::system_clock::now();
		dump_output(std::string_view{reinterpret_cast<char const*>(msg.payload.data()), msg.payload.size()});
	}
	void flush() override {};
	void set_pattern(std::string const&) override {};
	void set_formatter(std::unique_ptr<spdlog::formatter>) override {};
};

static void spdlog_test_combo(benchmark::State& state)
{
	auto test_sink = std::make_shared<dumpSink_spdlog>();
	spdlog::logger logger("multi_sink", {test_sink});

	for (auto _ : state)
	{
		logger.log(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info,
			"{0}{1}{2}{3}{4}", test_string, test_signed_int, test_unsigned_int, test_fp, test_char);
	}
}

static void spdlog_test_string(benchmark::State& state)
{
	auto test_sink = std::make_shared<dumpSink_spdlog>();
	spdlog::logger logger("multi_sink", {test_sink});
	for (auto _ : state)
	{
		logger.log(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, test_string);
	}
}

static void spdlog_test_null(benchmark::State& state)
{
	auto test_sink = std::make_shared<dumpSink_spdlog>();
	spdlog::logger logger("multi_sink", {test_sink});
	for (auto _ : state)
	{
		logger.log(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, "");
	}
}

BENCHMARK(spdlog_test_combo);
BENCHMARK(spdlog_test_string);
BENCHMARK(spdlog_test_null);

class dumpSink_spdlog_notime: public spdlog::sinks::sink
{
public:
	void log(spdlog::details::log_msg const& msg) override
	{
		dump_output(std::string_view{reinterpret_cast<char const*>(msg.payload.data()), msg.payload.size()});
	}
	void flush() override {};
	void set_pattern(std::string const&) override {};
	void set_formatter(std::unique_ptr<spdlog::formatter>) override {};
};

static void spdlog_test_combo_notime(benchmark::State& state)
{
	auto test_sink = std::make_shared<dumpSink_spdlog_notime>();
	spdlog::logger logger("multi_sink", {test_sink});

	for (auto _ : state)
	{
		logger.log(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info,
			"{0}{1}{2}{3}{4}", test_string, test_signed_int, test_unsigned_int, test_fp, test_char);
	}
}

static void spdlog_test_string_notime(benchmark::State& state)
{
	auto test_sink = std::make_shared<dumpSink_spdlog_notime>();
	spdlog::logger logger("multi_sink", {test_sink});
	for (auto _ : state)
	{
		logger.log(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, test_string);
	}
}

static void spdlog_test_null_notime(benchmark::State& state)
{
	auto test_sink = std::make_shared<dumpSink_spdlog_notime>();
	spdlog::logger logger("multi_sink", {test_sink});
	for (auto _ : state)
	{
		logger.log(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, "");
	}
}

BENCHMARK(spdlog_test_combo_notime);
BENCHMARK(spdlog_test_string_notime);
BENCHMARK(spdlog_test_null_notime);