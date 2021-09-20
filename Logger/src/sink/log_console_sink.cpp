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

#include <Logger/sink/log_console_sink.hpp>

#include <vector>

#include <CoreLib/Core_Console.hpp>
#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/Core_Alloca.hpp>

namespace logger
{

log_console_sink::log_console_sink() = default;

#ifdef _WIN32

static void finish_cout(const std::u8string_view p_level, const std::u8string_view p_message, char16_t* p_buffer, const uintptr_t p_size, const uintptr_t p_message_estimate, const bool p_printLevel)
{
	if(p_printLevel)
	{
		core::_p::ANSI_to_UCS2_unsafe(p_level, p_buffer);
		p_buffer += p_level.size();
	}

	if(p_message_estimate)
	{
		core::_p::UTF8_to_UTF16_faulty_unsafe(p_message, '?', p_buffer);
		p_buffer += p_message_estimate;
	}
	*p_buffer = u'\n';
	core::cout.write(std::u16string_view{p_buffer, p_size});
}

__declspec(noinline)
void log_console_sink::output(const log_data& p_logData)
{
	const bool print_level = (p_logData.m_levelNumber != Level::Info);
	const uintptr_t message_estimate = core::_p::UTF8_to_UTF16_faulty_estimate(p_logData.m_message, '?');
	const uintptr_t char_count =
		(print_level ? p_logData.m_level.size() : 0)
		+ message_estimate + 1;

	constexpr uintptr_t alloca_treshold = (0x10000 / sizeof(char16_t));

	if(char_count > alloca_treshold)
	{
		std::vector<char16_t> buff;
		buff.resize(char_count);
		finish_cout(p_logData.m_level, p_logData.m_message, buff.data(), char_count, message_estimate, print_level);
	}
	else
	{
		char16_t* buff = reinterpret_cast<char16_t*>(core_alloca(char_count * sizeof(char16_t)));
		finish_cout(p_logData.m_level, p_logData.m_message, buff, char_count, message_estimate, print_level);
	}
}

#else

static void finish_cout(const std::u8string_view p_level, const std::u8string_view p_message, char8_t* p_buffer, const uintptr_t p_size, const bool p_printLevel)
{
	if(p_printLevel)
	{
		const uintptr_t lsize = p_level.size();
		memcpy(p_buffer, p_level.data(), lsize);
		p_buffer +=lsize;
	}
	const uintptr_t msize = p_message.size();
	memcpy(p_buffer, p_message.data(), msize);
	p_buffer += msize;
	*p_buffer = u8'\n';
	core::cout.write(std::u8string_view{p_buffer, p_size});
}

__attribute__((noinline))
void log_console_sink::output(const log_data& p_logData)
{
	const bool print_level = (p_logData.m_levelNumber != Level::Info);
	const uintptr_t char_count =
		(print_level ? p_logData.m_level.size() : 0)
		+ p_logData.m_message.size() + 1;

	constexpr uintptr_t alloca_treshold = 0x10000;

	if(char_count > alloca_treshold)
	{
		std::vector<char8_t> buff;
		buff.resize(char_count);
		finish_cout(p_logData.m_level, p_logData.m_message, buff.data(), char_count, print_level);
	}
	else
	{
		char8_t* buff = reinterpret_cast<char8_t*>(core_alloca(char_count));
		finish_cout(p_logData.m_level, p_logData.m_message, buff, char_count, print_level);
	}
}
#endif

}
