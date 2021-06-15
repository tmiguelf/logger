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

#include "toLog.hpp"

#include <array>

#include <CoreLib/Core_Net.hpp>
#include <CoreLib/string/core_string_numeric.hpp>

namespace logger
{

template<>
class toLog<core::IPv4_netAddr>: public toLog_base
{
private:
	static constexpr uintptr_t max_ip_size = 15;
	using array_t = std::array<char8_t, max_ip_size>;
public:
	toLog(const core::IPv4_netAddr& p_data)
	{
		m_size = p_data.to_string(m_preCalc);
	}

	void push(char8_t* p_out) const final
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
};

template<>
class toLog<core::IPv6_netAddr>: public toLog_base
{
private:
	static constexpr uintptr_t max_ip_size = 39;
	using array_t = std::array<char8_t, max_ip_size>;
public:
	toLog(const core::IPv6_netAddr& p_data)
	{
		m_size = p_data.to_string(m_preCalc);
	}

	void push(char8_t* p_out) const final
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
};

template<>
class toLog<core::IP_netAddr>: public toLog_base
{
private:
	static constexpr uintptr_t max_ip_size = 39;
	using array_t = std::array<char8_t, max_ip_size>;
public:
	toLog(const core::IP_netAddr& p_data)
	{
		m_size = p_data.to_string(m_preCalc);
	}

	void push(char8_t* p_out) const final
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
};


template<typename T>
class toLog_net;
template <typename T> toLog_net(T, uint16_t) -> toLog_net<std::remove_cvref_t<T>>;

template<>
class toLog_net<core::IPv4_netAddr>: public toLog_base
{
private:
	static constexpr uintptr_t max_ip_size = 15;
	static constexpr uintptr_t max_port_size = core::to_chars_dec_max_digits_v<uint16_t>;
	using array_t = std::array<char8_t, max_ip_size + max_port_size + 1>;
public:
	toLog_net(const core::IPv4_netAddr& p_ip, uint16_t p_port)
	{
		const uintptr_t size1 = p_ip.to_string(std::span<char8_t, max_ip_size>{m_preCalc.data(), max_ip_size});
		m_preCalc[size1] = u8'@';
		m_size = size1 + 1 + core::to_chars(p_port, std::span<char8_t, max_port_size>{m_preCalc.data() + size1 + 1, max_port_size});
	}

	void push(char8_t* p_out) const final
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
};

template<>
class toLog_net<core::IPv6_netAddr>: public toLog_base
{
private:
	static constexpr uintptr_t max_ip_size = 39;
	static constexpr uintptr_t max_port_size = core::to_chars_dec_max_digits_v<uint16_t>;
	using array_t = std::array<char8_t, max_ip_size + max_port_size + 1>;
public:
	toLog_net(const core::IPv6_netAddr& p_ip, uint16_t p_port)
	{
		const uintptr_t size1 = p_ip.to_string(std::span<char8_t, max_ip_size>{m_preCalc.data(), max_ip_size});
		m_preCalc[size1] = u8'@';
		m_size = size1 + 1 + core::to_chars(p_port, std::span<char8_t, max_port_size>{m_preCalc.data() + size1 + 1, max_port_size});
	}

	void push(char8_t* p_out) const final
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
};

template<>
class toLog_net<core::IP_netAddr>: public toLog_base
{
private:
	static constexpr uintptr_t max_ip_size = 39;
	static constexpr uintptr_t max_port_size = core::to_chars_dec_max_digits_v<uint16_t>;
	using array_t = std::array<char8_t, max_ip_size + max_port_size + 1>;
public:
	toLog_net(const core::IP_netAddr& p_ip, uint16_t p_port)
	{
		const uintptr_t size1 = p_ip.to_string(std::span<char8_t, max_ip_size>{m_preCalc.data(), max_ip_size});
		m_preCalc[size1] = u8'@';
		m_size = size1 + 1 + core::to_chars(p_port, std::span<char8_t, max_port_size>{m_preCalc.data() + size1 + 1, max_port_size});
	}

	void push(char8_t* p_out) const final
	{
		memcpy(p_out, m_preCalc.data(), m_size);
	}

private:
	array_t m_preCalc;
};

} //namespace logger
