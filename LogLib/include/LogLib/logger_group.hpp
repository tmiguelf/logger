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

#include <string_view>
#include <vector>

#include <CoreLib/string/core_os_string.hpp>


namespace logger
{

class log_sink;
struct log_message_data;

/// \brief Log group class that holds Logger streamers such as Logging to File and Logging to Console
class LoggerGroup
{
	/// create list of references to Logger streamers
	std::vector<log_sink*> m_sinks;

public:

	///	\brief Send the log to the Log sink
	void log(log_message_data const& data, std::u8string_view message);

	///	\brief add the current log stream to the streams container
	///	param[in] p_stream - Log stream containg the log data
	void add_sink(log_sink& p_sink);

	///	\brief remove the current log stream from the streams container
	///	param[in] p_stream Log stream containing the log data
	void remove_sink(log_sink& p_sink);

	///	\breif clear streams container
	void clear();
};

}	// namespace simLog
