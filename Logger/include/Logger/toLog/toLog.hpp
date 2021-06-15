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
#include <type_traits>

namespace logger
{

class toLog_base
{
protected:
	uintptr_t m_size = 0;

public:
	constexpr toLog_base() = default;
	constexpr toLog_base(uintptr_t p_size): m_size{p_size}{}

	inline constexpr uintptr_t size() const { return m_size; }
	virtual void push(char8_t*) const = 0;
};


template <typename>
class toLog;
template <typename T> toLog(T) -> toLog<std::remove_cvref_t<T>>;

template<typename T>
class toLog_f: public toLog_base
{
public:
	using method_t = void (*)(const T&, char8_t*);
	using sizer_f = uintptr_t (*)(const T&);

public:
	inline constexpr toLog_f(const T& p_data, uintptr_t p_size, method_t p_method): toLog_base{p_size}, m_data{p_data}, m_method{p_method}{}
	inline constexpr toLog_f(const T& p_data, sizer_f p_sizer, method_t p_method): toLog_f(p_data, p_sizer(p_data), p_method){}

	void push(char8_t* p_out) const final
	{
		m_method(m_data, p_out);
	}

private:
	method_t m_method;
	const T& m_data;
};

template <typename T> toLog_f(T, uintptr_t, void(*)(const std::remove_cvref_t<T>&, char8_t*)) -> toLog_f<std::remove_cvref_t<T>>;
template <typename T> toLog_f(T, uintptr_t (*)(const std::remove_cvref_t<T>&), void(*)(const std::remove_cvref_t<T>&, char8_t*)) -> toLog_f<std::remove_cvref_t<T>>;

} //namespace logger