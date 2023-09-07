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

#pragma once

#include "Logger_client.hpp"
#include "toLog/log_streamer.hpp"

#include <CoreLib/toPrint/toPrint.hpp>
#include <CoreLib/core_module.hpp>



//======== ======== Macro Magic ======== ========
#ifdef _WIN32
#define LOG_CUSTOM(File, Line, Column, _Level, ...) \
	{ \
		const void* const _P_LOG_BASE_ADDR__ = ::core::get_current_module_base(); \
		const ::logger::Level _P_LOG_LEVEL__ = _Level; \
		if(::logger::_p::log_check_filter(_P_LOG_BASE_ADDR__, _P_LOG_LEVEL__, ::std::wstring_view{__FILEW__}, static_cast<uint32_t>(__LINE__))) \
		{ \
			core_ToPrint(char8_t, ::logger::_p::LogStreamer(_P_LOG_BASE_ADDR__, _P_LOG_LEVEL__, File, Line, Column), __VA_ARGS__); \
		} \
	}
#else
#define LOG_CUSTOM(File, Line, Column, _Level, ...) \
	{ \
		const void* const _P_LOG_BASE_ADDR__ = ::core::get_current_module_base(); \
		const ::logger::Level _P_LOG_LEVEL__ = _Level; \
		if(::logger::_p::log_check_filter(_P_LOG_BASE_ADDR__, _P_LOG_LEVEL__, ::std::string_view{__FILE__}, static_cast<uint32_t>(__LINE__))) \
		{ \
			core_ToPrint(char8_t, ::logger::_p::LogStreamer(_P_LOG_BASE_ADDR__, _P_LOG_LEVEL__, File, Line, Column), __VA_ARGS__); \
		} \
	}
#endif

/// \brief Helper Macro to assist on message formating and automatically filling of __FILE__ (__FILEW__ on windows) and __LINE__
/// \param[in] Level - \ref logger::Level

#ifdef _WIN32
#define LOG_MESSAGE(Level, ...) LOG_CUSTOM(::std::wstring_view{__FILEW__}, static_cast<uint32_t>(__LINE__), 0, Level, __VA_ARGS__)
#else
#define LOG_MESSAGE(Level, ...) LOG_CUSTOM(::std::string_view{__FILE__}, static_cast<uint32_t>(__LINE__), 0, Level, __VA_ARGS__)
#endif

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
