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

#include "Logger/Logger.hpp"
#include "Logger/Logger_service.hpp"
#include "Logger_p.hpp"

#include <span>
#include <array>

#include <CoreLib/Core_String.hpp>
#include <CoreLib/Core_Thread.hpp>

#if defined(_DEBUG) && defined(_WIN32)
#	include <Windows.h>
#	include <WinBase.h>
#	define __LOG_HAS_DEBUGGER
#endif	// _WIN32



//Right now we are enforcing validity by having buffer larger than what we would theorethically need
static constexpr uintptr_t g_DateTimeThreadMessageSize = sizeof("[00000/000/000-000:000:000.00000|0000000000]") - 1;
static inline bool formatDateTimeThreadValid(const core::DateTime&)
{
	//return !(	(p_time.date.month		>  12) ||
	//			(p_time.date.day		>  31) || 
	//			(p_time.time.hour		>  23) ||
	//			(p_time.time.minute		>  59) || 
	//			(p_time.time.second		>  59) || 
	//			(p_time.time.msecond	> 999)
	//		);
	return true;
}

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

static size_t FormatDateThread(const core::DateTime& p_time, core::thread_id_t p_threadId, std::span<char8_t, g_DateTimeThreadMessageSize> p_dateTimeThread)
{
	char8_t* pivot = p_dateTimeThread.data();

	if(formatDateTimeThreadValid(p_time))
	{
		//year
		*pivot = '['; ++pivot;
		pivot += core::to_chars(p_time.date.year, std::span<char8_t, 5>{pivot, 5});

		//month
		*pivot = '/'; ++pivot;
		if(p_time.date.month < 10) { *pivot = '0'; ++pivot; }
		pivot += core::to_chars(p_time.date.month, std::span<char8_t, 3>{pivot, 3});

		//day
		*pivot = '/'; ++pivot;
		if(p_time.date.day < 10) { *pivot = '0'; ++pivot; }
		pivot += core::to_chars(p_time.date.day, std::span<char8_t, 3>{pivot, 3});

		//hour
		*pivot = '-'; ++pivot;
		if(p_time.time.hour < 10) { *pivot = '0'; ++pivot; }
		pivot += core::to_chars(p_time.time.hour, std::span<char8_t, 3>{pivot, 3});

		//minute
		*pivot = ':'; ++pivot;
		if(p_time.time.minute < 10) { *pivot = '0'; ++pivot; }
		pivot += core::to_chars(p_time.time.minute, std::span<char8_t, 3>{pivot, 3});

		//second
		*pivot = ':'; ++pivot;
		if(p_time.time.second < 10) { *pivot = '0'; ++pivot; }
		pivot += core::to_chars(p_time.time.second, std::span<char8_t, 3>{pivot, 3});

		//millisecond
		*pivot = '.'; ++pivot;
		if(p_time.time.msecond < 100)
		{
			*pivot = u8'0'; ++pivot;
			if(p_time.time.msecond < 10) { *pivot = u8'0'; ++pivot; }
		}
		pivot += core::to_chars(p_time.time.msecond, std::span<char8_t, 5>{pivot, 5});
	}
	else
	{
		constexpr std::u8string_view invalidStr = u8"[N/A";
		memcpy(p_dateTimeThread.data(), invalidStr.data(), invalidStr.size());
		pivot += invalidStr.size();
	}

	//thread
	*pivot = '|'; ++pivot;
	constexpr uintptr_t thread_max_digits = core::to_chars_max_digits_v<core::thread_id_t>;
	pivot += core::to_chars(p_threadId, std::span<char8_t, thread_max_digits>{pivot, thread_max_digits});

	*pivot = ']'; ++pivot;
	return pivot - p_dateTimeThread.data();
}

static size_t FormatLogLevel(Level p_level,  std::span<char8_t, 9> p_out)
{
	switch(p_level)
	{
		case Level::Info:
			memcpy(p_out.data(), "Info: ", 6);
			return 6;
		case Level::Warning:
			memcpy(p_out.data(), "Warning: ", 9);
			return 9;
		case Level::Error:
			memcpy(p_out.data(), "Error: ", 7);
			return 7;
		case Level::Debug:
			memcpy(p_out.data(), "Debug: ", 7);
			return 7;
		default:
			break;
	}
	memcpy(p_out.data(), "Lvl(  ): ", 9);
	core::to_hex_chars_fix(static_cast<uint8_t>(p_level), p_out.subspan<4, 2>());
	return 9;
}

#ifdef __LOG_HAS_DEBUGGER
static void output2debugger(
	std::u8string_view p_file,
	std::u8string_view p_line,
	std::u8string_view p_dateTimeThread,
	std::u8string_view p_category,
	std::u8string_view p_message)
{
	_p::u8string_stream t_stream;
	t_stream << p_file << '(' << p_line << "): " << p_dateTimeThread << ' ' << p_category << p_message << '\n';
	OutputDebugStringA(reinterpret_cast<const char*>(t_stream.str().c_str()));
}
#endif

//======== ======== ======== ======== Class: LoggerHelper ======== ======== ======== ========


void LoggerHelper::log(Level p_level, std::u8string_view p_file, uint32_t p_line, std::u8string_view p_message)
{
#ifndef _DEBUG
	if(p_level == Level::Debug) return;
#endif

	log_data log_data;

	log_data.m_time		= core::date_time_UTC();
	log_data.m_threadId	= getCurrentThreadId();

	//date/time/thread
	std::array<char8_t, g_DateTimeThreadMessageSize> dateTimeThread;
	uintptr_t dateTimeThread_size = FormatDateThread(log_data.m_time, log_data.m_threadId, dateTimeThread);

	//line
	std::array<char8_t, 10> line;
	uintptr_t line_size = core::to_chars(p_line, std::span<char8_t, 10>{line});

	//category
	std::array<char8_t, 9> level;
	uintptr_t level_size = FormatLogLevel(p_level, level);

	log_data.m_level			= std::u8string_view(level.data(), level_size);
	log_data.m_dateTimeThread	= std::u8string_view(dateTimeThread.data(), dateTimeThread_size);
	log_data.m_file				= p_file;
	log_data.m_line				= std::u8string_view(line.data(), line_size);
	log_data.m_message			= p_message;

	log_data.m_levelNumber	= p_level;
	log_data.m_lineNumber	= p_line;

	for(log_sink* sink: m_sinks)
	{
		sink->output2stream(log_data);
	}

#ifdef __LOG_HAS_DEBUGGER
	output2debugger(
		p_file,
		std::u8string_view(line.data(), line_size),
		std::u8string_view(dateTimeThread.data(), dateTimeThread_size),
		std::u8string_view(level.data(), level_size),
		p_message);
#endif
}

void LoggerHelper::push(PreLogHelper& p_prelogs)
{
	log_data log_data;

	//date/time/thread
	std::array<char8_t, g_DateTimeThreadMessageSize> dateTimeThread;
	uintptr_t dateTimeThread_size = FormatDateThread(log_data.m_time, log_data.m_threadId, dateTimeThread);

	//line
	std::array<char8_t, 10> line;
	uintptr_t line_size;

	//category
	std::array<char8_t, 9> level;
	uintptr_t level_size;

	PreLogHelper safeContainer;
	safeContainer.swap(p_prelogs);

	for(PreLogUnit& logUnit : safeContainer)
	{

		//date/time/thread size
		dateTimeThread_size	= FormatDateThread(logUnit.time, logUnit.threadId, dateTimeThread);

		//line size
		line_size = core::to_chars(logUnit.line, std::span<char8_t, 10>{line});

		//category size
		level_size = FormatLogLevel(logUnit.level, level);

		log_data.m_level			= std::u8string_view(level.data(), level_size);
		log_data.m_dateTimeThread	= std::u8string_view(dateTimeThread.data(), dateTimeThread_size);
		log_data.m_file				= logUnit.file;
		log_data.m_line				= std::u8string_view(line.data(), line_size);
		log_data.m_message			= logUnit.message;

		log_data.m_levelNumber	= logUnit.level;
		log_data.m_lineNumber	= logUnit.line;
		log_data.m_threadId		= logUnit.threadId;
		log_data.m_time			= logUnit.time;

		for(log_sink* sink: m_sinks)
		{
			sink->output2stream(log_data);
		}
	}
}

void LoggerHelper::add_sink(log_sink& p_sink)
{
	m_sinks.push_back(&p_sink);
}

void LoggerHelper::remove_sink(log_sink& p_sink)
{
	log_sink* sink_addr = &p_sink;
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

Logger_API void Log_add_sink(log_sink& p_stream)
{
	g_logger.add_sink(p_stream);
}

Logger_API void Log_remove_sink(log_sink& p_stream)
{
	g_logger.remove_sink(p_stream);
}

}// namespace simLog


 //======== ======== ======== ======== Exported functions ======== ======== ======== ========

extern "C"
{

Logger_API void Log_Message(logger::Level p_level, const char8_t* p_file, uintptr_t p_fileNameSize, uint32_t p_line, const char8_t* p_message, uintptr_t p_messageSize)
{
	logger::g_logger.log(p_level, std::u8string_view{p_file, p_fileNameSize}, p_line, std::u8string_view{p_message, p_messageSize});
}

Logger_API void Log_Shutdown()
{
	logger::g_logger.clear();
}

[[nodiscard]] Logger_API PreLogHandle Log_CreatePreLogHandle()
{
	return static_cast<PreLogHandle>(new logger::PreLogHelper);
}

Logger_API void Log_DestroyPreLogHandle(PreLogHandle p_handle)
{
	if(p_handle)
	{
		delete static_cast<logger::PreLogHelper*>(p_handle);
	}
}

Logger_API void Log_PushPreLog(PreLogHandle p_handle)
{
	if(p_handle)
	{
		logger::g_logger.push(*static_cast<logger::PreLogHelper*>(p_handle));
	}
}

Logger_API void Log_2PreLog(PreLogHandle p_handle, logger::Level p_level, const char8_t* p_file, uintptr_t p_fileNameSize, uint32_t p_line, const char8_t* p_message, uintptr_t p_messageSize)
{
	std::u8string_view tfile	(p_file, p_fileNameSize);
	std::u8string_view tmessage	(p_message, p_messageSize);

	core::DateTime t_time = core::date_time_UTC();
	core::thread_id_t threadId = logger::getCurrentThreadId();

	if(p_handle)
	{
		logger::PreLogHelper* preLogs = static_cast<logger::PreLogHelper*>(p_handle);
		preLogs->emplace_back(logger::PreLogUnit{t_time, threadId, p_level, p_line, tfile, tmessage});
	}

#ifdef __LOG_HAS_DEBUGGER
	//date/time/thread
	std::array<char8_t, g_DateTimeThreadMessageSize> dateTimeThread;
	uintptr_t dateTimeThread_size = logger::FormatDateThread(t_time, threadId, dateTimeThread);

	//line
	std::array<char8_t, 10> line;
	uintptr_t line_size = core::to_chars(p_line, std::span<char8_t, 10>{line});

	//category
	std::array<char8_t, 9> level;
	uintptr_t level_size = FormatLogLevel(p_level, level);


	logger::output2debugger(
		tfile,
		std::u8string_view(line.data(), line_size),
		std::u8string_view(dateTimeThread.data(), dateTimeThread_size),
		std::u8string_view(level.data(), level_size),
		tmessage);
#endif
}

} //extern "C"
