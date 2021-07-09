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

#include "Logger_p.hpp"

#include <span>
#include <array>

#include <CoreLib/Core_Time.hpp>
#include <CoreLib/Core_Thread.hpp>
#include <CoreLib/string/core_os_string.hpp>
#include <CoreLib/string/core_string_numeric.hpp>

#include <Logger/Logger_client.hpp>
#include <Logger/Logger_service.hpp>
#include <Logger/sink/log_file_sink.hpp>

//Right now we are enforcing validity by having buffer larger than what we would theorethically need
static constexpr uintptr_t g_DateMessageSize = sizeof("00000/000/000") - 1;
static constexpr uintptr_t g_TimeMessageSize = sizeof("000:000:000.00000") - 1;


//#define BENCHMARK

/// \n
namespace logger
{

//======== ======== ======== ======== Global ======== ======== ======== ========
LoggerHelper g_logger; //!< Preserves settings


//======== ======== ======== ======== Format assists ======== ======== ======== ========

static core::thread_id_t getCurrentThreadId()
{
	thread_local static const core::thread_id_t threadId = core::current_thread_id();
	return threadId;
}

[[maybe_unused]] static uintptr_t FormatDate(const core::DateTime& p_time, std::span<char8_t, g_DateMessageSize> p_out)
{
	char8_t* pivot = p_out.data();

	//year
	pivot += core::to_chars(p_time.date.year, std::span<char8_t, 5>{pivot, 5});

	//month
	*(pivot++) = u8'/';
	if(p_time.date.month < 10) { *(pivot++) = u8'0'; }
	pivot += core::to_chars(p_time.date.month, std::span<char8_t, 3>{pivot, 3});

	//day
	*(pivot++) = u8'/';
	if(p_time.date.day < 10) { *(pivot++) = u8'0'; }
	pivot += core::to_chars(p_time.date.day, std::span<char8_t, 3>{pivot, 3});
	return pivot - p_out.data();
}

[[maybe_unused]] static uintptr_t FormatTime(const core::DateTime& p_time, std::span<char8_t, g_TimeMessageSize> p_out)
{
	char8_t* pivot = p_out.data();

	//hour
	if(p_time.time.hour < 10) { *(pivot++) = u8'0'; }
	pivot += core::to_chars(p_time.time.hour, std::span<char8_t, 3>{pivot, 3});

	//minute
	*(pivot++) = u8':';
	if(p_time.time.minute < 10) { *(pivot++) = u8'0'; }
	pivot += core::to_chars(p_time.time.minute, std::span<char8_t, 3>{pivot, 3});

	//second
	*(pivot++) = u8':';
	if(p_time.time.second < 10) { *(pivot++) = u8'0'; }
	pivot += core::to_chars(p_time.time.second, std::span<char8_t, 3>{pivot, 3});

	//millisecond
	*(pivot++) = u8'.';
	if(p_time.time.msecond < 100)
	{
		*(pivot++) = u8'0';
		if(p_time.time.msecond < 10) { *(pivot++) = u8'0'; }
	}
	pivot += core::to_chars(p_time.time.msecond, std::span<char8_t, 5>{pivot, 5});
	return pivot - p_out.data();
}


static size_t FormatLogLevel(Level p_level,  std::span<char8_t, 9> p_out)
{
	switch(p_level)
	{
		case Level::Info:
			{
				constexpr std::u8string_view text = u8"Info: ";
				memcpy(p_out.data(), text.data(), text.size());
				return text.size();
			}
		case Level::Warning:
			{
				constexpr std::u8string_view text = u8"Warning: ";
				memcpy(p_out.data(), text.data(), text.size());
				return text.size();
			}
		case Level::Error:
			{
				constexpr std::u8string_view text = u8"Error: ";
				memcpy(p_out.data(), text.data(), text.size());
				return text.size();
			}
		case Level::Debug:
			{
				constexpr std::u8string_view text = u8"Debug: ";
				memcpy(p_out.data(), text.data(), text.size());
				return text.size();
			}
		default:
			break;
	}

	{
		constexpr std::u8string_view text = u8"Lvl(  ): ";
		memcpy(p_out.data(), text.data(), text.size());
		core::to_chars_hex_fix(static_cast<uint8_t>(p_level), p_out.subspan<4, 2>());
		return text.size();
	}
}


//======== ======== ======== ======== Class: LoggerHelper ======== ======== ======== ========


void LoggerHelper::log(Level p_level, core::os_string_view p_file, uint32_t p_line, uint32_t p_column, std::u8string_view p_message)
{
	log_data log_data;

#ifndef BENCHMARK
	log_data.m_timeStruct	= core::date_time_UTC();

#endif // !BENCHMARK
	log_data.m_threadId		= getCurrentThreadId();

	//category
	std::array<char8_t, 9> level;
	const uintptr_t level_size = FormatLogLevel(p_level, level);

#ifndef BENCHMARK
	//date
	std::array<char8_t, g_DateMessageSize> date;
	const uintptr_t date_size = FormatDate(log_data.m_timeStruct, date);
	
	//time
	std::array<char8_t, g_TimeMessageSize> time;
	const uintptr_t time_size = FormatTime(log_data.m_timeStruct, time);
#endif // !BENCHMARK

	//thread
	std::array<char8_t, core::to_chars_dec_max_digits_v<core::thread_id_t>> thread;
	const uintptr_t thread_size = core::to_chars(log_data.m_threadId, thread);

	//line
	std::array<char8_t, 10> line;
	const uintptr_t line_size = core::to_chars(p_line, line);

	//column
	std::array<char8_t, 10> column;
	const uintptr_t column_size = core::to_chars(p_column, column);

	log_data.m_level	= std::u8string_view(level.data(), level_size);
#ifndef BENCHMARK
	log_data.m_date		= std::u8string_view(date.data(), date_size);
	log_data.m_time		= std::u8string_view(time.data(), time_size);
#endif // !BENCHMARK
	log_data.m_thread	= std::u8string_view(thread.data(), thread_size);
	log_data.m_file		= p_file;
	log_data.m_line		= std::u8string_view(line.data(), line_size);
	log_data.m_column	= std::u8string_view(column.data(), column_size);
	log_data.m_message	= p_message;

	log_data.m_levelNumber	= p_level;
	log_data.m_lineNumber	= p_line;
	log_data.m_columnNumber	= p_column;

	for(log_sink* const sink: m_sinks)
	{
		sink->output(log_data);
	}
}

void LoggerHelper::add_sink(log_sink& p_sink)
{
	m_sinks.push_back(&p_sink);
}

void LoggerHelper::remove_sink(log_sink& p_sink)
{
	log_sink* const sink_addr = &p_sink;
	for(decltype(m_sinks)::iterator it = m_sinks.begin(), it_end = m_sinks.end(); it != it_end; ++it)
	{
		if((*it) == sink_addr)
		{
			m_sinks.erase(it);
			return;
		}
	}
}

void LoggerHelper::clear()
{
	m_sinks.clear();
}

Logger_API void log_add_sink(log_sink& p_stream)
{
	g_logger.add_sink(p_stream);
}

Logger_API void log_remove_sink(log_sink& p_stream)
{
	g_logger.remove_sink(p_stream);
}

Logger_API void log_remove_all()
{
	g_logger.clear();
}

Logger_API void log_message(Level p_level, core::os_string_view p_file, uint32_t p_line, uint32_t p_column, std::u8string_view p_message)
{
	g_logger.log(p_level, p_file, p_line, p_column, p_message);
}

}// namespace logger
