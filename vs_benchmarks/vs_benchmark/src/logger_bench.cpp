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

#include <Logger/Logger.hpp>
#include <Logger/Logger_service.hpp>
#include <LogLib/sink/log_sink.hpp>


class dumpSink_logger: public logger::log_sink
{
public:
	void output(logger::log_data const& p_logData) override
	{
		dump_output(std::string_view{reinterpret_cast<char const*>(p_logData.message.data()), p_logData.message.size()});
	}
};

static void logger_test_combo(benchmark::State& state)
{
	dumpSink_logger tsink;
	logger::log_add_sink(tsink);
	for (auto _ : state)
	{
		LOG_INFO(test_string, test_signed_int,
			test_unsigned_int, test_fp, test_char);
	}
	logger::log_remove_all();
}

static void logger_test_string(benchmark::State& state)
{
	dumpSink_logger tsink;
	logger::log_add_sink(tsink);
	for (auto _ : state)
	{
		LOG_INFO(test_string);
	}
	logger::log_remove_all();
}

static void logger_test_null(benchmark::State& state)
{

	dumpSink_logger tsink;
	logger::log_add_sink(tsink);
	for (auto _ : state)
	{
		LOG_INFO();
	}
	logger::log_remove_all();
}

BENCHMARK(logger_test_combo);
BENCHMARK(logger_test_string);
BENCHMARK(logger_test_null);
