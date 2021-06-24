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
#include <cstring>
#include <concepts>
#include <type_traits>
#include <array>
#include <string_view>
#include <string>

#include "toLog.hpp"

#include <CoreLib/string/core_string_numeric.hpp>
#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/string/core_wchar_alias.hpp>

namespace logger
{
//======== ======== Pointers ======== ========

//-------- Disallow null terminated strings --------

namespace _p
{
	template <typename T>
	constexpr bool is_one_of_chars_v =
		std::is_same_v<T, char8_t > ||
		std::is_same_v<T, char16_t> ||
		std::is_same_v<T, char32_t> ||
		std::is_same_v<T, char    > ||
		std::is_same_v<T, wchar_t >;

	template <typename T>
	constexpr bool explicitly_disalowed_v =
		(std::is_array_v<T> && is_one_of_chars_v<std::remove_cvref_t<std::remove_all_extents_t<T>>>) ||
		(std::is_pointer_v<T> && is_one_of_chars_v<std::remove_cvref_t<std::remove_pointer_t<T>>>);

} //namespace _p

template<typename T> requires _p::explicitly_disalowed_v<T>
class toLog<T>: public toLog_base
{
public:
	toLog(const T&){}
	void push(char8_t*) const final;
	static_assert(!_p::explicitly_disalowed_v<T>, "Raw null terminated strings are explicitly disallowed. Please decide if it should be seen as a pointer or as a string_view");
};


//-------- Raw pointers --------

template<typename T> requires (std::is_pointer_v<T> && !_p::explicitly_disalowed_v<T>)
class toLog<T>: public toLog_base
{
private:
	static constexpr uintptr_t aux_size = core::to_chars_hex_max_digits_v<uintptr_t>;
	using array_t = std::array<char8_t, aux_size + 2>;

public:
	toLog(T p_data)
	{
		m_size = m_preCalc.size();
		m_preCalc[0] = '0';
		m_preCalc[1] = 'x';
		core::to_chars_hex_fix(reinterpret_cast<uintptr_t>(p_data),
			std::span<char8_t, aux_size>{m_preCalc.data() + 2, aux_size});
	}

	void push(char8_t* p_out) const final
	{
		memcpy(p_out, m_preCalc.data(), m_preCalc.size());
	}

private:
	array_t m_preCalc;
};


//======== ======== Strings ======== ========

//-------- Single char --------

template<>
class toLog<char8_t>: public toLog_base
{
public:
	constexpr toLog(char8_t p_data): toLog_base(1), m_data(p_data) {}

	void push(char8_t* p_out) const final
	{
		*p_out = m_data;
	}
private:
	char8_t m_data;
};


template<>
class toLog<char16_t>: public toLog_base
{
public:
	toLog(char16_t p_data)
	{
		const uintptr_t tsize = core::encode_UTF8(p_data, std::span<char8_t, 4>{m_preCalc.data(), 4});
		if(tsize)
		{
			m_size = tsize;
		}
		else
		{
			m_preCalc[0] = '?';
			m_size = 1;
		}

	}

	void push(char8_t* p_out) const final
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}
private:
	std::array<char8_t, 3> m_preCalc;
};


template<>
class toLog<char32_t>: public toLog_base
{
public:
	toLog(char32_t p_data)
	{
		const uintptr_t tsize = core::encode_UTF8(p_data, m_preCalc);
		if(tsize)
		{
			m_size = tsize;
		}
		else
		{
			m_preCalc[0] = '?';
			m_size = 1;
		}
	}

	void push(char8_t* p_out) const final
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}
private:
	std::array<char8_t, 4> m_preCalc;
};


template<>
class toLog<char>: public toLog<char8_t>
{
public:
	constexpr toLog(char p_data): toLog<char8_t>(static_cast<char8_t>(p_data))
	{}
};

template<>
class toLog<wchar_t>: public toLog<core::wchar_alias>
{
public:
	toLog(wchar_t p_data): toLog<core::wchar_alias>(static_cast<core::wchar_alias>(p_data))
	{}
};


//-------- String view --------

template<>
class toLog<std::u8string_view>: public toLog_base
{
public:
	toLog(std::u8string_view p_data): m_data(p_data)
	{
		m_size = m_data.size();
	}

	void push(char8_t* p_out) const final
	{
		memcpy(p_out, m_data.data(), m_data.size());
	}
private:
	std::u8string_view m_data;
};


template<>
class toLog<std::u16string_view>: public toLog_base
{
public:
	toLog(std::u16string_view p_data): m_data(p_data)
	{
		m_size = core::_p::UTF16_to_UTF8_faulty_estimate(p_data, '?');
	}

	void push(char8_t* p_out) const final
	{
		core::_p::UTF16_to_UTF8_faulty_unsafe(m_data, '?', p_out);
	}

private:
	std::u16string_view m_data;
};

template<>
class toLog<std::u32string_view>: public toLog_base
{
public:
	toLog(std::u32string_view p_data): m_data(p_data)
	{
		m_size = core::_p::UCS4_to_UTF8_faulty_estimate(p_data, '?');
	}

	void push(char8_t* p_out) const final
	{
		core::_p::UCS4_to_UTF8_faulty_unsafe(m_data, '?', p_out);
	}

private:
	std::u32string_view m_data;
};


template<>
class toLog<std::string_view>: public toLog<std::u8string_view>
{
public:
	toLog(std::string_view p_data)
		: toLog<std::u8string_view>(std::u8string_view{reinterpret_cast<const char8_t*>(p_data.data()), p_data.size()})
	{
	}
};

template<>
class toLog<std::wstring_view>: public toLog<std::basic_string_view<core::wchar_alias>>
{
public:
	toLog(std::wstring_view p_data)
		: toLog<std::basic_string_view<core::wchar_alias>>(
			std::basic_string_view<core::wchar_alias>{reinterpret_cast<const core::wchar_alias*>(p_data.data()), p_data.size()}
		)
	{
	}
};


//-------- String --------

template<typename T>
class toLog<std::basic_string<T>>: public toLog<std::basic_string_view<T>>
{
public:
	toLog(const std::basic_string<T>& p_data): toLog<std::basic_string_view<T>>(p_data) {}
};

//======== ======== Numeric ======== ========
namespace _p
{
	template<size_t TSIZE, size_t TALIGNED, bool TSIGNED>
	struct int_aliased_type;

	template<> struct int_aliased_type<sizeof(uint8_t ), alignof(uint8_t ), false> { using type = uint8_t ; };
	template<> struct int_aliased_type<sizeof(uint16_t), alignof(uint16_t), false> { using type = uint16_t; };
	template<> struct int_aliased_type<sizeof(uint32_t), alignof(uint32_t), false> { using type = uint32_t; };
	template<> struct int_aliased_type<sizeof(uint64_t), alignof(uint64_t), false> { using type = uint64_t; };
	template<> struct int_aliased_type<sizeof(int8_t ), alignof(int8_t ), true> { using type = int8_t ; };
	template<> struct int_aliased_type<sizeof(int16_t), alignof(int16_t), true> { using type = int16_t; };
	template<> struct int_aliased_type<sizeof(int32_t), alignof(int32_t), true> { using type = int32_t; };
	template<> struct int_aliased_type<sizeof(int64_t), alignof(int64_t), true> { using type = int64_t; };

	template<typename T>
	using int_aliased_type_t = int_aliased_type<sizeof(T), alignof(T), std::is_signed_v<T>>::type;
}
//-------- Decimal -------- 

template<typename Num_T> requires core::char_conv_dec_supported<Num_T>::value
class toLog<Num_T>: public toLog_base
{
private:
	using array_t = std::array<char8_t, core::to_chars_dec_max_digits_v<Num_T>>;
public:
	toLog(Num_T p_data)
	{
		m_size = core::to_chars(p_data, m_preCalc);
	}

	void push(char8_t* p_out) const final
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
};

template<typename Num_T> requires (std::integral<Num_T> && !core::char_conv_dec_supported<Num_T>::value)
class toLog<Num_T>: public toLog<_p::int_aliased_type_t<Num_T>>
{
	using alias_t = _p::int_aliased_type_t<Num_T>;
public:
	toLog(Num_T p_data): toLog<alias_t>(static_cast<alias_t>(p_data)) {}
};


//-------- Hexadecimal -------- 
template<typename>
class toLog_hex;
template <typename T> toLog_hex(T) -> toLog_hex<std::remove_cvref_t<T>>;

template<core::char_conv_hex_supported_c Num_T>
class toLog_hex<Num_T>: public toLog_base
{
private:
	using array_t = std::array<char8_t, core::to_chars_hex_max_digits_v<Num_T>>;

public:
	toLog_hex(Num_T p_data)
	{
		m_size = core::to_chars_hex(p_data, m_preCalc);
	}

	void push(char8_t* p_out) const final
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
};

template<typename Num_T> requires (std::integral<Num_T> && !core::char_conv_hex_supported<Num_T>::value)
class toLog_hex<Num_T>: public toLog_hex<_p::int_aliased_type_t<Num_T>>
{
	using alias_t = _p::int_aliased_type_t<Num_T>;
public:
	toLog_hex(Num_T p_data): toLog_hex<alias_t>(static_cast<alias_t>(p_data)) {}
};


//-------- Hexadecimal fixed size -------- 

template<typename>
class toLog_hex_fix;
template <typename T> toLog_hex_fix(T) -> toLog_hex_fix<std::remove_cvref_t<T>>;

template<core::char_conv_hex_supported_c Num_T>
class toLog_hex_fix<Num_T>: public toLog_base
{
private:
	static constexpr uintptr_t array_size = core::to_chars_hex_max_digits_v<Num_T>;

public:
	constexpr toLog_hex_fix(Num_T p_data): toLog_base(array_size), m_data{p_data} {}

	void push(char8_t* p_out) const final
	{
		core::to_chars_hex_fix(m_data, std::span<char8_t, array_size>{p_out, array_size});
	}

private:
	const Num_T m_data;
};

template<typename Num_T> requires (std::unsigned_integral<Num_T> && !core::char_conv_hex_supported<Num_T>::value)
class toLog_hex_fix<Num_T>: public toLog_hex_fix<_p::int_aliased_type_t<Num_T>>
{
	using alias_t = _p::int_aliased_type_t<Num_T>;
public:
	constexpr toLog_hex_fix(Num_T p_data): toLog_hex_fix<alias_t>(static_cast<alias_t>(p_data)) {}
};

} //namespace logger
