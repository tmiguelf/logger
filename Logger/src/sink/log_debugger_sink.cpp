//======== ======== ======== ======== ======== ======== ======== ========
///	\file
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

#include <Logger/sink/log_debugger_sink.hpp>

#ifdef _WIN32

#include <Windows.h>
#include <WinBase.h>

#include <sstream>
#include <string_view>
#include <vector>

#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/Core_Alloca.hpp>

namespace logger
{

static inline void AuxWriteData(
	const log_data& p_logData, char16_t* const p_buffer,
	uintptr_t p_line_estimate,
	uintptr_t p_col_estimate,
	uintptr_t p_date_estimate,
	uintptr_t p_level_estimate,
	uintptr_t p_message_estimate)
{
	char16_t* pivot = p_buffer;

	memcpy(pivot, p_logData.m_file.data(), p_logData.m_file.size());
	pivot += p_logData.m_file.size();

	*(pivot++) = u'(';

	core::_p::ANSI_to_UCS2_unsafe(p_logData.m_line, pivot);
	pivot += p_line_estimate;

	if(p_logData.m_columnNumber)
	{
		core::_p::ANSI_to_UCS2_unsafe(p_logData.m_column, pivot);
		pivot += p_col_estimate;
	}

	*(pivot++) = u')';
	*(pivot++) = u':';
	*(pivot++) = u' ';

	core::_p::ANSI_to_UCS2_unsafe(p_logData.m_dateTimeThread, pivot);
	pivot += p_date_estimate;

	*(pivot++) = u' ';

	core::_p::ANSI_to_UCS2_unsafe(p_logData.m_level, pivot);
	pivot += p_level_estimate;

	std::basic_stringstream<char16_t> t_stream;
	t_stream << reinterpret_cast<const std::u16string_view&>(p_logData.m_file) << u'('
		<< core::ANSI_to_UTF16(p_logData.m_line);


	core::_p::UTF8_to_UTF16_faulty_unsafe(p_logData.m_message, '?', pivot);
	pivot += p_message_estimate;

	*(pivot++) = u'\n';
	*pivot = 0;
	OutputDebugStringW(reinterpret_cast<const wchar_t*>(p_buffer));

}

log_debugger_sink::log_debugger_sink() = default;

void log_debugger_sink::output(const log_data& p_logData)
{
	const uintptr_t line_estimate = core::_p::ANSI_to_UCS2_estimate(p_logData.m_line);
	const uintptr_t col_estimate = p_logData.m_columnNumber ? core::_p::ANSI_to_UCS2_estimate(p_logData.m_line) : 0;
	const uintptr_t date_estimate = core::_p::ANSI_to_UCS2_estimate(p_logData.m_dateTimeThread);
	const uintptr_t level_estimate = core::_p::ANSI_to_UCS2_estimate(p_logData.m_level);
	const uintptr_t message_estimate = core::_p::UTF8_to_UTF16_faulty_estimate(p_logData.m_message, '?');

	//File(Line,Col): [Date] Level: Message\n\0
	const uintptr_t count = p_logData.m_file.size()
		+ line_estimate
		+ (p_logData.m_columnNumber ? col_estimate + 1 : 0) //,
		+ date_estimate
		+ level_estimate
		+ message_estimate
		+ 7; //():  \n\0


	constexpr uintptr_t alloca_treshold = 0x8000;

	if(count > alloca_treshold)
	{
		std::vector<char16_t> buff;
		buff.resize(count);
		AuxWriteData(p_logData, buff.data(),
			line_estimate, col_estimate, date_estimate, level_estimate, message_estimate);
	}
	else
	{
		char16_t* buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
		AuxWriteData(p_logData, buff,
			line_estimate, col_estimate, date_estimate, level_estimate, message_estimate);
	}

}

} //namespace logger
#endif // _WIN32
