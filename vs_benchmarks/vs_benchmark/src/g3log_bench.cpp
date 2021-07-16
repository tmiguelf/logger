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
#include "common.hpp"
#include <string>
#include <memory>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <g3log/logmessage.hpp>

class dumpSink_g3log
{
public:
	void ReceiveLogMessage(g3::LogMessageMover logEntry)
	{
		dump_output(logEntry.get().toString());
	}
};

static void g3log_test_combo(benchmark::State& state)
{
	std::unique_ptr<g3::LogWorker> logworker{ g3::LogWorker::createLogWorker() };
	[[maybe_unused]] auto sinkHandle = logworker->addSink(std::make_unique<dumpSink_g3log>(),
		&dumpSink_g3log::ReceiveLogMessage);

	g3::initializeLogging(logworker.get());

	for (auto _ : state)
	{
		LOG(INFO) << test_string << test_signed_int << test_unsigned_int << test_fp << test_char;
	}

	g3::internal::shutDownLogging();
}

static void g3log_test_string(benchmark::State& state)
{
	std::unique_ptr<g3::LogWorker> logworker{ g3::LogWorker::createLogWorker() };
	[[maybe_unused]] auto sinkHandle = logworker->addSink(std::make_unique<dumpSink_g3log>(),
		&dumpSink_g3log::ReceiveLogMessage);

	g3::initializeLogging(logworker.get());

	for (auto _ : state)
	{
		LOG(INFO) << test_string;
	}
	g3::internal::shutDownLogging();
}

static void g3log_test_null(benchmark::State& state)
{
	std::unique_ptr<g3::LogWorker> logworker{ g3::LogWorker::createLogWorker() };
	[[maybe_unused]] auto sinkHandle = logworker->addSink(std::make_unique<dumpSink_g3log>(),
		&dumpSink_g3log::ReceiveLogMessage);

	g3::initializeLogging(logworker.get());

	for (auto _ : state)
	{
		LOG(INFO);
	}
	g3::internal::shutDownLogging();
}

BENCHMARK(g3log_test_combo);
BENCHMARK(g3log_test_string);
BENCHMARK(g3log_test_null);
