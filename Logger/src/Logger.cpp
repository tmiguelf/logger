//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///
///	\copyright
///		Copyright (c) Tiago Miguel Oliveira Freire
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
#include <Logger/log_filter.hpp>
#include <Logger/sink/log_file_sink.hpp>

//Right now we are enforcing validity by having buffer larger than what we would theorethically need
static constexpr uintptr_t g_DateMessageSize = sizeof("00000/00/00") - 1;
static constexpr uintptr_t g_TimeMessageSize = sizeof("00:00:00.000") - 1;

/// \n
namespace logger
{

//======== ======== ======== ======== Global ======== ======== ======== ========
static LoggerHelper g_logger; //!< Preserves settings
static log_filter const* g_filter = nullptr;
static bool g_default_filter_behaviour = true;

//======== ======== ======== ======== Format assists ======== ======== ======== ========

static core::thread_id_t getCurrentThreadId()
{
	thread_local static const core::thread_id_t threadId = core::current_thread_id();
	return threadId;
}

[[maybe_unused]]
static uintptr_t FormatDate(const core::date_time& p_time, std::span<char8_t, g_DateMessageSize> const p_out)
{
	char8_t* pivot = p_out.data();

	//year
	pivot += core::to_chars(p_time.date.year, std::span<char8_t, 5>{pivot, 5}) + 6;

	uintptr_t const size = pivot - p_out.data();

	//day
	*(--pivot) = u8'0' + p_time.date.day % 10;
	*(--pivot) = u8'0' + p_time.date.day / 10;
	*(--pivot) = u8'/';

	//month
	*(--pivot) = u8'0' + p_time.date.month % 10; 
	*(--pivot) = u8'0' + p_time.date.month / 10; 
	*(--pivot) = u8'/';

	return size;
}

[[maybe_unused]]
static void FormatTime(const core::date_time& p_time, std::span<char8_t, g_TimeMessageSize> const p_out)
{
	char8_t* pivot = p_out.data() + 11;

	//millisecond
	*(pivot) = u8'0' + p_time.time.msecond % 10;
	{
		const char8_t rem = static_cast<char8_t>(p_time.time.msecond / 10);
		*(--pivot) = u8'0' + rem % 10;
		*(--pivot) = u8'0' + rem / 10;
	}
	*(--pivot) = u8'.';

	//second
	*(--pivot) = u8'0' + p_time.time.second % 10;
	*(--pivot) = u8'0' + p_time.time.second / 10;
	*(--pivot) =  u8':';

	//minute
	*(--pivot) = u8'0' + p_time.time.minute % 10;
	*(--pivot) = u8'0' + p_time.time.minute / 10;
	*(--pivot) = u8':';

	//hour
	*(--pivot) = u8'0' + p_time.time.hour % 10;
	*(--pivot) = u8'0' + p_time.time.hour / 10;
}

[[maybe_unused]]
static uintptr_t FormatLogLevel(const Level p_level, std::span<char8_t, 9> const p_out)
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

void LoggerHelper::log(void const* p_moduleBase, const Level p_level, const core::os_string_view p_file, const uint32_t p_line, const uint32_t p_column, const std::u8string_view p_message)
{
	log_data log_data;

	core::date_time_UTC(log_data.m_timeStruct);

	//category
	std::array<char8_t, 9> level;
	const uintptr_t level_size = FormatLogLevel(p_level, level);

	//date
	std::array<char8_t, g_DateMessageSize> date;
	const uintptr_t date_size = FormatDate(log_data.m_timeStruct, date);
	
	//time
	std::array<char8_t, g_TimeMessageSize> time;
	constexpr uintptr_t time_size = 12; FormatTime(log_data.m_timeStruct, time);
	//thread
	std::array<char8_t, core::to_chars_dec_max_size_v<core::thread_id_t>> thread;
	const uintptr_t thread_size = core::to_chars(log_data.m_threadId, thread);

	//line
	std::array<char8_t, 10> line;
	const uintptr_t line_size = core::to_chars(p_line, line);

	//column
	std::array<char8_t, 10> column;
	const uintptr_t column_size = core::to_chars(p_column, column);

	log_data.m_file			= p_file;
	log_data.m_line			= std::u8string_view(line.data(), line_size);
	log_data.m_column		= std::u8string_view(column.data(), column_size);
	log_data.m_date			= std::u8string_view(date.data(), date_size);
	log_data.m_time			= std::u8string_view(time.data(), time_size);
	log_data.m_thread		= std::u8string_view(thread.data(), thread_size);
	log_data.m_level		= std::u8string_view(level.data(), level_size);
	log_data.m_message		= p_message;

	log_data.m_moduleBase	= p_moduleBase;
	log_data.m_threadId		= getCurrentThreadId();
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
	for(decltype(m_sinks)::const_iterator it = m_sinks.cbegin(), it_end = m_sinks.cend(); it != it_end; ++it)
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


//======== ======== ======== ======== Public API ======== ======== ======== ========

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

Logger_API void log_message(void const* const p_moduleBase, const Level p_level, const core::os_string_view p_file, const uint32_t p_line, const uint32_t p_column, const std::u8string_view p_message)
{
	g_logger.log(p_moduleBase, p_level, p_file, p_line, p_column, p_message);
}

Logger_API void log_set_filter(log_filter const& p_filter)
{
	g_filter = &p_filter;
}

Logger_API void log_reset_filter(bool p_default_behaviour)
{
	g_filter = nullptr;
	g_default_filter_behaviour = p_default_behaviour;
}

namespace _p
{

Logger_API bool log_check_filter(void const* const p_moduleBase, const Level p_level, const core::os_string_view p_file, const uint32_t p_line)
{
	if(g_filter)
	{
		return g_filter->filter(p_moduleBase, p_level, p_file, p_line);
	}
	return g_default_filter_behaviour;
}

} //namespace _p

}// namespace logger
