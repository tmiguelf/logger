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

#include <Logger/toLog/log_streamer.hpp>

#include <vector>

#include <CoreLib/Core_Alloca.hpp>

namespace logger::_p
{

void LogStreamer::push(std::span<const toLog_base*> p_data, char8_t* const p_buff, uintptr_t p_size) const
{
	char8_t* pivot = p_buff;
	for(const toLog_base* t_obj : p_data)
	{
		const uintptr_t size = t_obj->size();
		if(size)
		{
			t_obj->push(pivot);
			pivot += size;
		}
	}

	log_message(m_level, m_file, m_line, m_column, std::u8string_view{p_buff, p_size});
}


Logger_API void LogStreamer::push(uintptr_t p_char_count, std::span<const toLog_base*> p_data) const
{
	constexpr uintptr_t alloca_treshold = 0x10000;

	if(p_char_count > alloca_treshold)
	{
			std::vector<char8_t> buff;
			buff.resize(p_char_count);
			push(p_data, buff.data(), p_char_count);
	}
	else
	{
		char8_t* buff = reinterpret_cast<char8_t*>(core_alloca(p_char_count));
		push(p_data, buff, p_char_count);
	}
}

} //namespace logger::_p
