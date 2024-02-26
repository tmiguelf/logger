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

#include <CoreLib/core_time.hpp>
#include <CoreLib/core_thread.hpp>
#include <CoreLib/string/core_os_string.hpp>

#include <Logger/log_level.hpp>


namespace logger
{
///	\brief Holds the Logging data information
struct log_data
{
	core::os_string_view	m_file;
	std::u8string_view		m_line;
	std::u8string_view		m_column;
	std::u8string_view		m_date;
	std::u8string_view		m_time;
	std::u8string_view		m_thread;
	std::u8string_view		m_level;
	std::u8string_view		m_message;

	void const*				m_moduleBase;
	core::thread_id_t		m_threadId;
	uint32_t				m_lineNumber;
	uint32_t				m_columnNumber;
	core::date_time			m_timeStruct;
	Level					m_levelNumber;
};

///	\brief Created to do Logging streams
class log_sink
{
public:
	virtual void output(const log_data& p_logData) = 0;
};

}	// namespace simLog
