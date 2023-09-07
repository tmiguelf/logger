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
#include <span>
#include <vector>

#include <CoreLib/toPrint/toPrint_sink.hpp>

#include <Logger/log_level.hpp>
#include <Logger/Logger_client.hpp>

namespace logger::_p
{
	class LogStreamer: public core::sink_toPrint_base
	{
	private:
		void const* const m_moduleBase; 
		const core::os_string_view m_file;
		const uint32_t m_line;
		const uint32_t m_column;
		const Level m_level;

	public:
		constexpr inline LogStreamer(
			void const* const p_moduleBase,
			const Level p_level,
			const core::os_string_view p_file,
			const uint32_t p_line,
			const uint32_t p_column)
			: m_moduleBase	(p_moduleBase)
			, m_level		(p_level)
			, m_file		(p_file)
			, m_line		(p_line)
			, m_column		(p_column)
		{
		}

		void write(const std::u8string_view p_message) const
		{
			::logger::log_message(m_moduleBase, m_level, m_file, m_line, m_column, p_message);
		}
	};

	inline constexpr void no_op() {}
} //namespace logger::_p

