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

#pragma once

#include <cstdint>
#include <string_view>

#include <CoreLib/core_time.hpp>
#include <CoreLib/core_thread.hpp>
#include <CoreLib/string/core_os_string.hpp>

#include "../logger_struct.hpp"

namespace logger
{
///	\brief Holds the Logging data information
struct log_data: public log_message_data
{
	inline log_data() = default;
	inline log_data(log_message_data const& p_other)
		: log_message_data(p_other)
	{
	}

	core::thread_id_t		thread_id;
	core::date_time_t		time_struct;

	std::u8string_view		message;
	std::u8string_view		sv_line;
	std::u8string_view		sv_column;
	std::u8string_view		sv_date;
	std::u8string_view		sv_time;
	std::u8string_view		sv_thread;
	std::u8string_view		sv_level;
};

///	\brief Created to do Logging streams
class log_sink
{
public:
	virtual void output(log_data const& p_logData) = 0;
};

}	// namespace simLog
