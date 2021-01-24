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

#pragma once

#include <cstdint>

#include <CoreLib/string/core_os_string.hpp>
#include <CoreLib/string/core_os_string.hpp>
#include <CoreLib/Core_Type.hpp>

#include "log_streamer.hpp"
#include "log_level.hpp"


#include "Logger_api.h"

//======== ======== API ======== ========

namespace logger
{
///	\brief Public interface for logging
///	\param[in] p_level - \ref logger::Level
///	\param[in] p_file - Name of source file generating the log. Pleas use the Macro __FILE__
///	\param[in] p_line - Source file line number where the Log was generated. Please use the Macro __LINE__
///	\param[in] p_column - Source file column number. Use 0 to signify the whole line.
///	\param[in] p_message - Message to Log
Logger_API void Log_Message(Level p_level, core::os_string_view p_file, uint32_t p_line, uint32_t p_column, std::u8string_view p_message);

namespace _p
{

class LogHelper
{
private:
	const core::os_string_view m_file;
	const uint32_t m_line;
	const uint32_t m_column;
	const Level m_level;

public:
	constexpr inline LogHelper(core::os_string_view p_file, uint32_t p_line, uint32_t p_column, Level p_level)
		: m_file	(p_file)
		, m_line	(p_line)
		, m_column	(p_column)
		, m_level	(p_level)
	{
	}

	inline void operator = (const LogStreamer& p_stream)
	{
		Log_Message(m_level, m_file, m_line, m_column, p_stream.stream().str());
	}
};

} //namespace _p
} //namespace logger


//======== ======== Macro Magic ======== ========

#define LOG_CUSTOM(File, Line, Column, Level) logger::_p::LogHelper{File, Line, Column, Level} = logger::_p::LogStreamer{}

/// \brief Helper Macro to assist on message formating and automatically filling of __FILE__ (__FILEW__ on windows) and __LINE__
/// \param[in] Level - \ref logger::Level

#ifdef _WIN32
#define LOG_MESSAGE(Level) LOG_CUSTOM(std::wstring_view{__FILEW__}, static_cast<uint32_t>(__LINE__), 0, Level)
#else
#define LOG_MESSAGE(Level) LOG_CUSTOM(std::string_view{__FILE__}, static_cast<uint32_t>(__LINE__), 0, Level)
#endif

/// \brief Helper Macro for info logs
#define LOG_INFO	LOG_MESSAGE(logger::Level::Info)

/// \brief Helper Macro for warning logs
#define LOG_WARNING	LOG_MESSAGE(logger::Level::Warning)

/// \brief Helper Macro for error logs
#define LOG_ERROR	LOG_MESSAGE(logger::Level::Error)

#ifdef _DEBUG
/// \brief Helper Macro for debug logs. If built in debug mode, message will be logged, if built in release the Macro will automatically eliminate the lines of code.
#	define LOG_DEBUG	LOG_MESSAGE(logger::Level::Debug)
#else
/// \brief Helper Macro for debug logs. If built in debug mode, message will be logged, if built in release the Macro will automatically eliminate the lines of code.
#	define LOG_DEBUG	logger::_p::DumpStream{}
#endif
