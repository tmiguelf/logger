//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///
///	\author Tiago Freire
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <CoreLib/Core_Thread.hpp>
#include <CoreLib/Core_Type.hpp>

#include <Logger/Logger.hpp>
#include <Logger/Logger_service.hpp>
#include <Logger/log_sink.hpp>

#include <iostream>
#include <vector>
#include <array>

using namespace core::literals;

struct log_cache
{
#ifdef _WIN32
	std::u16string file;
#else
	std::u8string file;
#endif
	std::u8string lineStr;
	std::u8string dateTimeThread;
	std::u8string levelStr;
	std::u8string message;

	core::DateTime		time;
	core::thread_id_t	threadId;
	uint32_t			line;
	logger::Level		level;
};

class test_sink: public logger::log_sink
{
	void output2stream(const logger::log_data& p_logData)
	{
		log_cache& cache = m_log_cache.emplace_back();

		cache.file				= p_logData.m_file;
		cache.lineStr			= p_logData.m_line;
		cache.dateTimeThread	= p_logData.m_dateTimeThread;
		cache.levelStr			= p_logData.m_level;
		cache.message			= p_logData.m_message;
		cache.time				= p_logData.m_time;
		cache.threadId			= p_logData.m_threadId;
		cache.line				= p_logData.m_lineNumber;
		cache.level				= p_logData.m_levelNumber;
	}
public:

	std::vector<log_cache> m_log_cache;
};


class TestStr
{
public:
	uint64_t data = 0;
};

inline std::ostream& operator << (std::ostream& p_stream, const TestStr& p_data)
{
	p_stream << "TestStr = " << core::toStream{p_data.data};
	return p_stream;
}

TEST(Logger, Logger_interface)
{
	{
		test_sink tsink;
		logger::Log_add_sink(tsink);

		TestStr test;
#ifdef _WIN32
		std::wstring_view help{__FILEW__};

		core::os_string_view fileName = core::rvalue_reinterpret_cast<core::os_string_view>(std::wstring_view{__FILEW__});
#else
		core::os_string_view fileName = core::rvalue_reinterpret_cast<core::os_string_view>(std::string_view{__FILE__});
#endif

		std::vector<uint32_t> logLines;
		core::thread_id_t threadId = core::current_thread_id();

		logLines.push_back(static_cast<uint32_t>(__LINE__)); LOG_INFO << test;
		logLines.push_back(static_cast<uint32_t>(__LINE__)); LOG_WARNING << 32;
		logLines.push_back(static_cast<uint32_t>(__LINE__)); LOG_ERROR << &test;
		logLines.push_back(static_cast<uint32_t>(__LINE__)); LOG_INFO << "char string";
		logLines.push_back(static_cast<uint32_t>(__LINE__)); LOG_INFO << u8"char8_t string";
		logLines.push_back(static_cast<uint32_t>(__LINE__)); LOG_INFO << std::string_view{"string_view"};
		logLines.push_back(static_cast<uint32_t>(__LINE__)); LOG_INFO << std::u8string_view{u8"u8string_view"};
		logLines.push_back(static_cast<uint32_t>(__LINE__)); LOG_INFO << 'A';
		logLines.push_back(static_cast<uint32_t>(__LINE__)); LOG_INFO << u8'A';
		logLines.push_back(static_cast<uint32_t>(__LINE__)); LOG_INFO << uint8_t{5};
		logLines.push_back(static_cast<uint32_t>(__LINE__)); LOG_INFO << int8_t{-5};
		logLines.push_back(static_cast<uint32_t>(__LINE__)); LOG_INFO << "Combination " << 32 << ' ' << &test;

		logger::Log_remove_sink(tsink);

		ASSERT_EQ(tsink.m_log_cache.size(), logLines.size());

		std::array<char8_t, 10> buff;

		for(uintptr_t i = 0; i < logLines.size(); ++i)
		{
			log_cache& cache = tsink.m_log_cache[i];
			std::u8string_view lineStr{buff.data(), core::to_chars(logLines[i], std::span<char8_t, 10>{buff})};

			ASSERT_EQ(cache.line, logLines[i]) << "Case " << i;
			ASSERT_EQ(cache.lineStr, lineStr) << "Case " << i;
			ASSERT_EQ(cache.file, fileName) << "Case " << i;
			ASSERT_EQ(cache.threadId, threadId) << "Case " << i;
			//TODO: nned improvement
			ASSERT_FALSE(cache.dateTimeThread.empty()) << "Case " << i; //Note might need better test
			//cache.time
		}

		ASSERT_EQ(tsink.m_log_cache[0].level, logger::Level::Info);
		ASSERT_EQ(tsink.m_log_cache[0].levelStr, std::u8string_view{u8"Info: "});
		ASSERT_EQ(tsink.m_log_cache[1].level, logger::Level::Warning);
		ASSERT_EQ(tsink.m_log_cache[1].levelStr, std::u8string_view{u8"Warning: "});
		ASSERT_EQ(tsink.m_log_cache[2].level, logger::Level::Error);
		ASSERT_EQ(tsink.m_log_cache[2].levelStr, std::u8string_view{u8"Error: "});

		ASSERT_EQ(tsink.m_log_cache[0].message, std::u8string_view{u8"TestStr = 0"});
		ASSERT_EQ(tsink.m_log_cache[1].message, std::u8string_view{u8"32"});
		{
			std::u8string_view log_message_2 = tsink.m_log_cache[2].message;
			ASSERT_EQ(log_message_2.size(), sizeof(void*) * 2 + 2);
			ASSERT_EQ(log_message_2.substr(0, 2), std::u8string_view{u8"0x"});
			ASSERT_TRUE(core::is_hex(log_message_2.substr(2)));
		}
		ASSERT_EQ(tsink.m_log_cache[3].message, std::u8string_view{u8"char string"});
		ASSERT_EQ(tsink.m_log_cache[4].message, std::u8string_view{u8"char8_t string"});
		ASSERT_EQ(tsink.m_log_cache[5].message, std::u8string_view{u8"string_view"});
		ASSERT_EQ(tsink.m_log_cache[6].message, std::u8string_view{u8"u8string_view"});
		ASSERT_EQ(tsink.m_log_cache[7].message, std::u8string_view{u8"A"});
		ASSERT_EQ(tsink.m_log_cache[8].message, std::u8string_view{u8"A"});
		ASSERT_EQ(tsink.m_log_cache[9].message, std::u8string_view{u8"5"});
		ASSERT_EQ(tsink.m_log_cache[10].message, std::u8string_view{u8"-5"});
	}

	{
		test_sink tsink;
		logger::Log_add_sink(tsink);
		LOG_DEBUG << "Debug Test " << 32 << ' ';
		logger::Log_remove_sink(tsink);
#ifdef _DEBUG
		ASSERT_FALSE(tsink.m_log_cache.empty());
#else
		ASSERT_TRUE(tsink.m_log_cache.empty());
#endif
	}

	{
		test_sink tsink;
		logger::Log_add_sink(tsink);
#ifdef _WIN32
		const std::u16string fileName {u"Random Name"};
#else
		const std::u8string fileName {u8"Random Name"};
#endif

		LOG_CUSTOM(fileName, 42, logger::Level{0x12}) << "Custom Test " << 32 << ' ';

		logger::Log_remove_sink(tsink);
		ASSERT_EQ(tsink.m_log_cache.size(), 1_uip);

		log_cache& cache = tsink.m_log_cache[0];

		ASSERT_EQ(cache.level, logger::Level{0x12});
		ASSERT_EQ(cache.levelStr, std::u8string_view{u8"Lvl(12): "});
		ASSERT_EQ(cache.line, 42_ui32);
		ASSERT_EQ(cache.lineStr, std::u8string_view{u8"42"});
		ASSERT_EQ(cache.file, fileName);
	}
}

