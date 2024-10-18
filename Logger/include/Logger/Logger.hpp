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

#pragma once

#include "Logger_client.hpp"
#include "toLog/log_streamer.hpp"

#include <LogLib/logger_struct.hpp>

#include <CoreLib/string/core_os_string.hpp>
#include <CoreLib/toPrint/toPrint.hpp>
#include <CoreLib/core_module.hpp>


//======== ======== Macro Magic ======== ========

#ifdef _WIN32
#define __LOG_FILE __FILEW__

#else
#define __LOG_FILE __FILE__
#endif

#define LOG_CUSTOM(File, Line, Column, _Level, ...) \
	{ \
		::logger::log_message_data _P_BASE_LOG_DATA; \
		_P_BASE_LOG_DATA.module_base = ::core::get_current_module_base(); \
		_P_BASE_LOG_DATA.user_token  = nullptr; \
		_P_BASE_LOG_DATA.module_name = ::core::get_current_module_name(); \
		_P_BASE_LOG_DATA.file        = ::core::os_string_view{__LOG_FILE}; \
		_P_BASE_LOG_DATA.line        = static_cast<uint32_t>(__LINE__); \
		_P_BASE_LOG_DATA.column      = 0; \
		_P_BASE_LOG_DATA.level       = _Level; \
		if(::logger::_p::log_check_filter(_P_BASE_LOG_DATA)) \
		{ \
			_P_BASE_LOG_DATA.file   = File; \
			_P_BASE_LOG_DATA.line   = Line; \
			_P_BASE_LOG_DATA.column = Column; \
			core::print<char8_t>(::logger::_p::LogStreamer(_P_BASE_LOG_DATA) __VA_OPT__(,) __VA_ARGS__); \
		} \
	}

/// \brief Helper Macro to assist on message formating and automatically filling of __FILE__ (__FILEW__ on windows) and __LINE__
/// \param[in] Level - \ref logger::Level
#define LOG_MESSAGE(Level, ...) LOG_CUSTOM(::core::os_string_view{__LOG_FILE}, static_cast<uint32_t>(__LINE__), 0, Level, __VA_ARGS__)

/// \brief Helper Macro for info logs
#define LOG_INFO(...)		LOG_MESSAGE(::logger::Level::Info, __VA_ARGS__)

/// \brief Helper Macro for warning logs
#define LOG_WARNING(...)	LOG_MESSAGE(::logger::Level::Warning, __VA_ARGS__)

/// \brief Helper Macro for error logs
#define LOG_ERROR(...)		LOG_MESSAGE(::logger::Level::Error, __VA_ARGS__)

#ifdef _DEBUG
/// \brief Helper Macro for debug logs. If built in debug mode, message will be logged, if built in release the Macro will automatically eliminate the lines of code.
#	define LOG_DEBUG(...)	LOG_MESSAGE(::logger::Level::Debug, __VA_ARGS__)
#else
/// \brief Helper Macro for debug logs. If built in debug mode, message will be logged, if built in release the Macro will automatically eliminate the lines of code.
#	define LOG_DEBUG(...)	::logger::_p::no_op();
#endif
