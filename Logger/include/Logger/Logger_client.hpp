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

#include <cstdint>
#include <string_view>

#include <CoreLib/string/core_os_string.hpp>

#include "log_level.hpp"
#include "Logger_api.h"

//======== ======== API ======== ========

namespace logger
{
///	\brief Public interface for logging
///	\param[in] p_moduleBase - Based address of the module generating the log
///	\param[in] p_level - \ref logger::Level
///	\param[in] p_file - Name of source file generating the log. Pleas use the Macro __FILE__
///	\param[in] p_line - Source file line number where the Log was generated. Please use the Macro __LINE__
///	\param[in] p_column - Source file column number. Use 0 to signify the whole line.
///	\param[in] p_message - Message to Log
Logger_API void log_message(void const* p_moduleBase, Level p_level, core::os_string_view p_file, uint32_t p_line, uint32_t p_column, std::u8string_view p_message);

namespace _p
{
	[[nodiscard]] Logger_API bool log_check_filter(void const* p_moduleBase, Level p_level, core::os_string_view p_file, uint32_t p_line);
} //namespace _p

} //namespace logger