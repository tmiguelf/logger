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

#include "disk_g3log.hpp"

#include <g3log/g3log.hpp>
#include <g3log/filesink.hpp>

#include <g3log/logworker.hpp>


namespace disk_g3log
{
	std::unique_ptr<g3::LogWorker> logworker;
	std::unique_ptr<g3::FileSinkHandle> sinkHandle;
	void testSetup()
	{
		logworker = g3::LogWorker::createLogWorker();
		sinkHandle = logworker->addDefaultLogger("log", "", "g3log");
		g3::initializeLogging(logworker.get());
	}

	void log(std::string_view p_str, int32_t p_int32, uint64_t p_uint64, double p_double, char p_char)
	{
		LOG(INFO) << p_str << p_int32 << p_uint64 << p_double << p_char;
	}

	void testClean()
	{
		g3::internal::shutDownLogging();
		logworker.reset();
		sinkHandle.reset();
	}
}
