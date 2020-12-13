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

#pragma once

#include <cstdint>
#include <string_view>
#include <sstream>
#include <type_traits>

#include <fstream>

#include <CoreLib/string/core_string_tostream.hpp>
#include <CoreLib/string/core_string_streamers.hpp>

namespace logger::_p
{
	using u8string_stream = std::basic_stringstream<char8_t>;


	template <typename, typename, typename = void>
	struct is_stream_op_available : std::false_type {};

	template <typename charT, typename opT>
	struct is_stream_op_available<charT, opT,
		std::void_t<decltype(operator << (std::declval<std::basic_stringstream<charT>>(), 
			std::declval<const opT&>())) >> : std::true_type {};

	template<typename charT, typename OpT>
	constexpr bool has_stream_operator = is_stream_op_available<charT, OpT>::value;

	class LogStreamer
	{
	public:
		inline LogStreamer& operator << (std::string_view p_message)
		{
			m_stream.write(reinterpret_cast<const char8_t*>(p_message.data()), p_message.size());
			return *this;
		}

		inline LogStreamer& operator << (std::u8string_view p_message)
		{
			m_stream.write(p_message.data(), p_message.size());
			return *this;
		}

		inline LogStreamer& operator << (char p_data)
		{
			m_stream.put(reinterpret_cast<char8_t&>(p_data));
			return *this;
		}

		inline LogStreamer& operator << (char8_t p_data)
		{
			m_stream.put(p_data);
			return *this;
		}

		inline LogStreamer& operator << (const char* p_data)
		{
			return operator << (std::string_view{p_data});
		}

		inline LogStreamer& operator << (const char8_t* p_data)
		{
			return operator << (std::u8string_view{p_data});
		}

		template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
		inline LogStreamer& operator << (T p_data)
		{
			reinterpret_cast<std::basic_stringstream<char>&>(m_stream) << core::toStream(p_data);
			return *this;
		}

		template<typename T, std::enable_if_t<std::is_pointer_v<T>, int> = 0>
		inline LogStreamer& operator << (T p_data)
		{
			reinterpret_cast<std::basic_stringstream<char>&>(m_stream) << core::toStream<void*>(p_data);
			return *this;
		}

		template<typename T, std::enable_if_t<
			!std::is_arithmetic_v<T> &&
			!std::is_pointer_v<T> &&
			has_stream_operator<char8_t, T>
			, int> = 0>
		inline LogStreamer& operator << (const T& p_data)
		{
			m_stream << p_data;
			return *this;
		}

		template<typename T, std::enable_if_t<
			!std::is_arithmetic_v<T> &&
			!std::is_pointer_v<T> &&
			!has_stream_operator<char8_t, T> &&
			has_stream_operator<char, T>
			, int> = 0>
		inline LogStreamer& operator << (const T& p_data)
		{
			reinterpret_cast<std::basic_stringstream<char>&>(m_stream) << p_data;
			return *this;
		}

		template<typename T, std::enable_if_t<
			!std::is_arithmetic_v<T> &&
			!std::is_pointer_v<T> &&
			!has_stream_operator<char8_t, T> &&
			!has_stream_operator<char, T>
			, int> = 0>
		inline void operator << (const T&)
		{
			static_assert(std::is_arithmetic_v<T> ||
				std::is_pointer_v<T> ||
				has_stream_operator<char8_t, T> ||
				has_stream_operator<char, T>
				, "No suitable stream operation defined");
		}

		[[nodiscard]] inline const u8string_stream& stream() const { return m_stream; }
	private:
		u8string_stream m_stream;
	};


	template <typename, typename = void>
	struct stream_supports : std::false_type {};

	template <typename opT>
	struct stream_supports<opT,
		std::enable_if_t<
		!std::is_same_v<decltype(std::declval<LogStreamer>() << std::declval<const opT&>()), void>, void>>
		: std::true_type {};

	class DumpStream
	{
	public:
		template<typename T, std::enable_if_t<stream_supports<T>::value, int> = 0>
		inline DumpStream& operator << (const T&)
		{
			return *this;
		}

		template<typename T, std::enable_if_t<!stream_supports<T>::value, int> = 0>
		inline void operator << (const T&)
		{
			static_assert(stream_supports<T>::value, "No suitable stream operation defined");
		}
	};

} //namespace logger::_p

