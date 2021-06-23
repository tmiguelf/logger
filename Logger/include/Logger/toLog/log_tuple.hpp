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

#include <tuple>
#include <type_traits>

#include <string>
#include <string_view>

#include "toLog.hpp"

namespace logger::_p
{
	template<typename> struct is_tuple : public std::false_type {};
	template<typename ...Type> struct is_tuple<std::tuple<Type...>> : public std::true_type{};

	template<typename T>
	concept c_tuple = is_tuple<T>::value;


	template<typename Derived, typename Base>
	constexpr bool is_derived_v = std::is_base_of_v<Base, Derived> && std::is_convertible_v<const volatile Derived*, const volatile Base*>;

	template<typename T>
	constexpr bool is_toLog_v = is_derived_v<T, ::logger::toLog_base>;

	//template<typename T>
	//concept c_toLog = is_toLog_v<T>;

	template <c_tuple Tuple>
	struct tuple_toLog
	{
		static constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;

		template<uintptr_t>
		struct sub;

		template<>
		struct sub<tuple_size>
		{
			using type = std::tuple<>;
		};

		template<uintptr_t pos>
		struct sub
		{
			using evaluated_t = std::remove_cvref_t<decltype(std::get<pos>(std::declval<Tuple>()))>;

			using type = decltype(std::tuple_cat(std::declval<std::tuple<std::conditional_t<is_toLog_v<evaluated_t>, const evaluated_t&, toLog<evaluated_t>>>>(), std::declval<sub<pos + 1>::type>()));
		};

	public:
		using type = sub<0>::type; //todo
	};


	template <c_tuple Tuple>
	struct is_all_toLog
	{
		static constexpr uintptr_t tuple_size = std::tuple_size_v<Tuple>;

		template<uintptr_t>
		struct sub;

		template<>
		struct sub<tuple_size>
		{
			static constexpr bool value = true;
		};

		template<uintptr_t pos>
		struct sub
		{
			using evaluated_t = std::remove_cvref_t<decltype(std::get<pos>(std::declval<Tuple>()))>;

			static constexpr bool value = is_toLog_v<evaluated_t> && sub<pos + 1>::value;
		};

	public:
		static constexpr bool value = sub<0>::value;
	};


	template<typename> struct is_tuple_toLog : public std::false_type {};
	template<c_tuple T> struct is_tuple_toLog<T> : public is_all_toLog<T>{};

	template<typename T>
	concept c_tuple_toLog = is_tuple_toLog<T>::value;


	template <c_tuple Tuple>
	struct tuple_toLog_or_string_view
	{
		using type = tuple_toLog<Tuple>::type;
	};

	template <>
	struct tuple_toLog_or_string_view <std::tuple<std::u8string_view>>
	{
		using type = std::u8string_view;
	};

	template <>
	struct tuple_toLog_or_string_view <std::tuple<std::u8string>>
	{
		using type = std::u8string_view;
	};

	template <>
	struct tuple_toLog_or_string_view <std::tuple<std::string_view>>
	{
		using type = std::string_view;
	};

	template <>
	struct tuple_toLog_or_string_view <std::tuple<std::string>>
	{
		using type = std::string_view;
	};

} //namespace logger::_p
