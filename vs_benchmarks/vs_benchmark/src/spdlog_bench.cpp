#include "common.hpp"

#include <spdlog/spdlog.h>




class dumpSink_spdlog: public spdlog::sinks::sink
{
public:
	void log(const spdlog::details::log_msg& msg) override
	{
		dump_output(std::string_view{reinterpret_cast<const char*>(msg.payload.data()), msg.payload.size()});
	}
	void flush() override {};
	void set_pattern(const std::string&) override {};
	void set_formatter(std::unique_ptr<spdlog::formatter>) override {};
};

static void spdlog_test_combo(benchmark::State& state)
{
	auto test_sink = std::make_shared<dumpSink_spdlog>();
	spdlog::logger logger("multi_sink", {test_sink});
	for (auto _ : state)
	{
		logger.info("{0}{1}{2}{3}{4}", test_string, test_signed_int, test_unsigned_int, test_fp, test_char);
	}
}

static void spdlog_test_string(benchmark::State& state)
{
	auto test_sink = std::make_shared<dumpSink_spdlog>();
	spdlog::logger logger("multi_sink", {test_sink});
	for (auto _ : state)
	{
		logger.info("{0}", test_string);
	}
}

static void spdlog_test_null(benchmark::State& state)
{
	auto test_sink = std::make_shared<dumpSink_spdlog>();
	spdlog::logger logger("multi_sink", {test_sink});
	for (auto _ : state)
	{
		logger.info("");
	}
}

BENCHMARK(spdlog_test_combo);
BENCHMARK(spdlog_test_string);
BENCHMARK(spdlog_test_null);
