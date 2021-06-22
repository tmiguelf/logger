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

#pragma once

#include <cstdint>
#include <span>

#include <CoreLib/string/core_os_string.hpp>

#include <Logger/Logger_api.h>
#include <Logger/log_level.hpp>
#include <Logger/Logger_client.hpp>
#include "toLog.hpp"
#include "log_tuple.hpp"


namespace logger::_p
{
	class LogStreamer
	{
	private:
		const Level m_level;
		const core::os_string_view m_file;
		const uint32_t m_line;
		const uint32_t m_column;

		Logger_API void push(uintptr_t p_char_count, std::span<const toLog_base*> p_data) const;

		void push(std::span<const toLog_base*> p_data, char8_t* const p_buff, uintptr_t p_size) const;


		template <uintptr_t Pos, uintptr_t Size, typename Tuple>
		static inline uintptr_t fill(const Tuple& p_tuple, std::array<const toLog_base*, Size>& p_data)
		{
			const auto& res = std::get<Pos>(p_tuple);
			p_data[Pos] = &res;

			if constexpr (Pos + 1 < Size)
			{
				return res.size() + fill<Pos + 1, Size>(p_tuple, p_data);
			}
			else if constexpr (Pos + 1 == Size)
			{
				return res.size();
			}
			else
			{
				return 0;
			}
		}

	public:
		constexpr inline LogStreamer(Level p_level, core::os_string_view p_file, uint32_t p_line, uint32_t p_column)
			: m_level	(p_level)
			, m_file	(p_file)
			, m_line	(p_line)
			, m_column	(p_column)
		{
		}

		template<c_tuple_toLog Tuple>
		void log(const Tuple& p_data) const
		{
			constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;
			if constexpr (tuple_size > 0)
			{
				std::array<const toLog_base*, tuple_size> out_data;
				const uintptr_t char_count = fill<0, tuple_size, Tuple>(p_data, out_data);
				if(char_count > 0)
				{
					push(char_count, out_data);
					return;
				}
			}
			::logger::log_message(m_level, m_file, m_line, m_column, std::u8string_view{nullptr, 0});
		};
	};

	inline constexpr void no_op() {}
} //namespace logger::_p

