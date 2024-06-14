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

#include "disk_spdlog.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace disk_spdlog
{

#ifdef _WIN32
	auto test_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(L"spdlog.log");
#else
	auto test_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>("spdlog.log");
#endif
	spdlog::logger logger("multi_sink", test_sink);

	void testSetup()
	{
	}

	void log(std::string_view p_str, int32_t p_int32, uint64_t p_uint64, double p_double, char p_char)
	{
		logger.log(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, 
			"{0}{1}{2}{3}{4}", p_str, p_int32, p_uint64, p_double, p_char);
	}

	void testClean()
	{
	}
}
