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

#include "Logger/log_debugger_sink.hpp"

#if _WIN32

#include <Windows.h>
#include <WinBase.h>

#include <sstream>
#include <string_view>

namespace logger
{

log_debugger_sink::log_debugger_sink() = default;

void log_debugger_sink::output(const log_data& p_logData)
{
	std::basic_stringstream<char16_t> t_stream;
	t_stream << reinterpret_cast<const std::u16string_view&>(p_logData.m_file) << u'('
		<< core::ANSI_to_UTF16(p_logData.m_line);

	if(p_logData.m_columnNumber)
	{
		t_stream << u',' << core::ANSI_to_UTF16(p_logData.m_column);
	}

	t_stream << u"): "
		<< core::ANSI_to_UTF16(p_logData.m_dateTimeThread)
		<< u' ' << core::ANSI_to_UTF16(p_logData.m_level)
		<< core::ANSI_to_UTF16(p_logData.m_message) << u'\n';
	OutputDebugStringW(reinterpret_cast<const wchar_t*>(t_stream.str().c_str()));
}

} //namespace logger
#endif // _WIN32
