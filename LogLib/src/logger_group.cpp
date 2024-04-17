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

#include <LogLib/logger_group.hpp>

#include <span>
#include <array>

#include <CoreLib/core_time.hpp>
#include <CoreLib/core_thread.hpp>
#include <CoreLib/string/core_os_string.hpp>
#include <CoreLib/string/core_string_numeric.hpp>

#include <LogLib/log_filter.hpp>
#include <LogLib/logger_struct.hpp>
#include <LogLib/sink/log_sink.hpp>


//Right now we are enforcing validity by having buffer larger than what we would theorethically need
static constexpr uintptr_t g_DateMessageSize = sizeof("00000/00/00") - 1;
static constexpr uintptr_t g_TimeMessageSize = sizeof("00:00:00.000") - 1;

/// \n
namespace logger
{

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
	*(pivot) = u8'0' + static_cast<char8_t>(p_time.time.msecond % 10);
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

void LoggerGroup::log(log_message_data const& data, std::u8string_view message)
{
	log_data tlog_data = data;

	core::date_time_UTC(tlog_data.time_struct);
	tlog_data.thread_id = getCurrentThreadId();
	tlog_data.message = message;
	//category
	std::array<char8_t, 9> level;
	const uintptr_t level_size = FormatLogLevel(data.level, level);

	//date
	std::array<char8_t, g_DateMessageSize> date;
	const uintptr_t date_size = FormatDate(tlog_data.time_struct, date);
	
	//time
	std::array<char8_t, g_TimeMessageSize> time;
	constexpr uintptr_t time_size = 12; FormatTime(tlog_data.time_struct, time);
	//thread
	std::array<char8_t, core::to_chars_dec_max_size_v<core::thread_id_t>> thread;
	const uintptr_t thread_size = core::to_chars(tlog_data.thread_id, thread);

	//line
	std::array<char8_t, 10> line;
	const uintptr_t line_size = core::to_chars(data.line, line);

	//column
	std::array<char8_t, 10> column;
	const uintptr_t column_size = core::to_chars(data.column, column);

	tlog_data.sv_line   = std::u8string_view(line  .data(), line_size);
	tlog_data.sv_column = std::u8string_view(column.data(), column_size);
	tlog_data.sv_date   = std::u8string_view(date  .data(), date_size);
	tlog_data.sv_time   = std::u8string_view(time  .data(), time_size);
	tlog_data.sv_thread = std::u8string_view(thread.data(), thread_size);
	tlog_data.sv_level  = std::u8string_view(level .data(), level_size);

	for(log_sink* const sink: m_sinks)
	{
		sink->output(tlog_data);
	}
}

void LoggerGroup::add_sink(log_sink& p_sink)
{
	m_sinks.push_back(&p_sink);
}

void LoggerGroup::remove_sink(log_sink& p_sink)
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

void LoggerGroup::clear()
{
	m_sinks.clear();
}

}// namespace logger
