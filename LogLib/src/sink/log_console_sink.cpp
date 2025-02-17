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

#include <LogLib/sink/log_console_sink.hpp>

#include <vector>

#include <CoreLib/core_console.hpp>
#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/core_alloca.hpp>
#include <CoreLib/core_extra_compiler.hpp>

namespace logger
{

log_console_sink::log_console_sink() = default;


static void finish_cout(std::u8string_view const p_level, std::u8string_view const p_message, char8_t* p_buffer, uintptr_t const p_size, bool const p_printLevel)
{
	if(p_printLevel)
	{
		uintptr_t const lsize = p_level.size();
		memcpy(p_buffer, p_level.data(), lsize);
		p_buffer += lsize;
		*(p_buffer++) = u8':';
		*(p_buffer++) = u8' ';
	}
	uintptr_t const msize = p_message.size();
	memcpy(p_buffer, p_message.data(), msize);
	p_buffer += msize;
	*p_buffer = u8'\n';
	core::cout.write(std::u8string_view{p_buffer, p_size});
}

NO_INLINE void log_console_sink::output(log_data const& p_logData)
{
	bool const print_level = (p_logData.level != Level::Info);
	uintptr_t const char_count =
		(print_level ? p_logData.sv_level.size() + 2 : 0)
		+ p_logData.message.size() + 1;

	constexpr uintptr_t alloca_treshold = 0x10000;

	if(char_count > alloca_treshold)
	{
		std::vector<char8_t> buff;
		buff.resize(char_count);
		finish_cout(p_logData.sv_level, p_logData.message, buff.data(), char_count, print_level);
	}
	else
	{
		char8_t* buff = reinterpret_cast<char8_t*>(core_alloca(char_count));
		finish_cout(p_logData.sv_level, p_logData.message, buff, char_count, print_level);
	}
}

}
