#include "common.hpp"

#include <Logger/Logger.hpp>
#include <Logger/Logger_service.hpp>
#include <Logger/sink/log_sink.hpp>


class dumpSink_logger: public logger::log_sink
{
public:
	void output(const logger::log_data& p_logData) override
	{
		dump_output(std::string_view{reinterpret_cast<const char*>(p_logData.m_message.data()), p_logData.m_message.size()});
	}
};

static void logger_test_combo(benchmark::State& state)
{
	dumpSink_logger tsink;
	logger::log_add_sink(tsink);

	using test_t = std::remove_cvref_t<decltype(::logger::_p::LogStreamer{logger::Level::Info, L"Test", 32, 1})>;


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
