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

#include <LogLib/sink/log_debugger_sink.hpp>

#ifdef _WIN32

#include <Windows.h>

#include <string_view>
#include <vector>

#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/core_alloca.hpp>
#include <CoreLib/core_extra_compiler.hpp>

namespace logger
{

static inline void AuxWriteData(
	log_data const& p_logData, char16_t* const p_buffer,
	uintptr_t const p_line_estimate,
	uintptr_t const p_col_estimate,
	uintptr_t const p_time_estimate,
	uintptr_t const p_thread_estimate,
	uintptr_t const p_level_estimate,
	uintptr_t const p_message_estimate)
{
	char16_t* pivot = p_buffer;

	memcpy(pivot, p_logData.file.data(), p_logData.file.size() * sizeof(core::os_char));
	pivot += p_logData.file.size();

	*(pivot++) = u'(';

	core::ANSI_to_UCS2_unsafe(p_logData.sv_line, pivot);
	pivot += p_line_estimate;

	if(p_logData.column)
	{
		core::ANSI_to_UCS2_unsafe(p_logData.sv_column, pivot);
		pivot += p_col_estimate;
	}

	*(pivot++) = u')';
	*(pivot++) = u':';
	*(pivot++) = u' ';
	*(pivot++) = u'[';

	core::ANSI_to_UCS2_unsafe(p_logData.sv_time, pivot);
	pivot += p_time_estimate;
	*(pivot++) = u'|';
	core::ANSI_to_UCS2_unsafe(p_logData.sv_thread, pivot);
	pivot += p_thread_estimate;
	*(pivot++) = u']';
	*(pivot++) = u' ';

	core::ANSI_to_UCS2_unsafe(p_logData.sv_level, pivot);
	pivot += p_level_estimate;


	core::UTF8_to_UTF16_faulty_unsafe(p_logData.message, '?', pivot);
	pivot += p_message_estimate;

	*(pivot++) = u'\n';
	*pivot = 0;
	OutputDebugStringW(reinterpret_cast<wchar_t const*>(p_buffer));
}

log_debugger_sink::log_debugger_sink() = default;

NO_INLINE void log_debugger_sink::output(log_data const& p_logData)
{
	if(IsDebuggerPresent())
	{
		uintptr_t const line_estimate		= core::ANSI_to_UCS2_size(p_logData.sv_line);
		uintptr_t const col_estimate		= p_logData.column ? core::ANSI_to_UCS2_size(p_logData.sv_column) : 0;
		uintptr_t const time_estimate		= core::ANSI_to_UCS2_size(p_logData.sv_time);
		uintptr_t const thread_estimate		= core::ANSI_to_UCS2_size(p_logData.sv_thread);
		uintptr_t const level_estimate		= core::ANSI_to_UCS2_size(p_logData.sv_level);
		uintptr_t const message_estimate	= core::UTF8_to_UTF16_faulty_size(p_logData.message, '?');

		//File(Line,Col): [Date] Level: Message\n\0
		uintptr_t const count = p_logData.file.size()
			+ line_estimate
			+ (p_logData.column ? col_estimate + 1 : 0) //,
			+ time_estimate
			+ thread_estimate
			+ level_estimate
			+ message_estimate
			+ 10; //(): [|] \n\0

		constexpr uintptr_t alloca_treshold = 0x8000;

		if(count > alloca_treshold)
		{
			std::vector<char16_t> buff;
			buff.resize(count);
			AuxWriteData(p_logData, buff.data(),
				line_estimate, col_estimate,
				time_estimate, thread_estimate,
				level_estimate, message_estimate);
		}
		else
		{
			char16_t* buff = reinterpret_cast<char16_t*>(core_alloca(count * sizeof(char16_t)));
			AuxWriteData(p_logData, buff,
				line_estimate, col_estimate,
				time_estimate, thread_estimate,
				level_estimate, message_estimate);
		}
	}
}

} //namespace logger
#endif // _WIN32
