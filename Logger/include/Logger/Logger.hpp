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

#include "log_streamer.hpp"

#include "Logger_api.h"

/// \n
namespace logger
{
/// \brief Log categories
enum class Level: uint8_t
{
	Info	= 0x00,	//!< Info level
	Warning	= 0x01,	//!< Warning level
	Error	= 0x02,	//!< Error level
	Debug	= 0xFF	//!< Debug only, will not be available on realease build
};
} //namespace logger

//======== ======== API ======== ========

extern "C"
{
	///	\brief Public interface for logging
	///	\param[in] p_level			- \ref logger::Level
	///	\param[in] p_file			- Name of source file generating the log. Please use the Macro __FILE__
	///	\param[in] p_fileNameSize	- Lenght of p_file string
	///	\param[in] p_line			- Source file line number where the Log was generated. Please use the Macro __LINE__
	///	\param[in] p_message		- Message to Log
	///	\param[in] p_messageSize	- Lenght of p_message
	Logger_API void Log_Message(logger::Level p_level, const char8_t* p_file, uintptr_t p_fileNameSize, uint32_t p_line, const char8_t* p_message, uintptr_t p_messageSize);
}	//extern "C"


namespace logger
{
///	\brief Public interface for logging
///	\param[in] p_level - \ref logger::Level
///	\param[in] p_file - Name of source file generating the log. Pleas use the Macro __FILE__
///	\param[in] p_line - Source file line number where the Log was generated. Please use the Macro __LINE__
///	\param[in] p_message - Message to Log
inline void Log_Message(Level p_level, std::u8string_view p_file, uint32_t p_line, std::u8string_view p_message)
{
	::Log_Message(p_level, p_file.data(), p_file.size(), p_line, p_message.data(), p_message.size());
}

namespace _p
{

class LogHelper
{
private:
	const Level m_level;
	const std::string_view m_file;
	const uint32_t m_line;

public:
	constexpr inline LogHelper(Level p_level, std::string_view p_file, uint32_t p_line)
		: m_level	(p_level)
		, m_file	(p_file)
		, m_line	(p_line)
	{
	}

	inline void operator = (const LogStreamer& p_stream)
	{
		Log_Message(m_level, reinterpret_cast<const std::u8string_view&>(m_file), m_line, p_stream.stream().str());
	}
};

} //namespace _p
} //namespace logger


//======== ======== Macro Magic ======== ========

#define LOG_CUSTOM(File, Line, Level) logger::_p::LogHelper{Level, File, static_cast<uint32_t>(Line)} = logger::_p::LogStreamer{}

/// \brief Helper Macro to assist on message formating and automatically filling of __FILE__ and __LINE__
/// \param[in] Level - \ref logger::Level
#define LOG_MESSAGE(Level) LOG_CUSTOM(__FILE__, __LINE__, Level)

/// \brief Helper Macro for info logs
#define LOG_INFO		LOG_MESSAGE(logger::Level::Info)

/// \brief Helper Macro for warning logs
#define LOG_WARNING	LOG_MESSAGE(logger::Level::Warning)

/// \brief Helper Macro for error logs
#define LOG_ERROR		LOG_MESSAGE(logger::Level::Error)

#ifdef _DEBUG
/// \brief Helper Macro for debug logs. If built in debug mode, message will be logged, if built in release the Macro will automatically eliminate the lines of code.
#	define LOG_DEBUG	LOG_MESSAGE(logger::Level::Debug)
#else
/// \brief Helper Macro for debug logs. If built in debug mode, message will be logged, if built in release the Macro will automatically eliminate the lines of code.
#	define LOG_DEBUG	logger::_p::DumpStream{}
#endif
