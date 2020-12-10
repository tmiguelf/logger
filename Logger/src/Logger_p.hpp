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

#include "Logger/Logger.hpp"
#include "Logger/log_sink.hpp"

#include <deque>
#include <string>
#include <string_view>

#include <CoreLib/Core_Thread.hpp>
#include <CoreLib/Core_Time.hpp>

#include <vector>


/// \n
namespace logger
{

/// \brief Log helper class that holds Logger streamers such as Logging to File and Logging to Console
class LoggerHelper
{
	/// create list of references to Logger streamers
	std::vector<log_sink*> m_sinks;

public:

	///	\brief Send the log to the Log sink
	///	\param[in] p_category - \ref logger::Level
	///	\param[in] p_file - Null terminated, Name of source file generating the log.
	///	\param[in] p_line - Source file line number where the Log was generated.
	///	\param[in] p_message - Null terminated message to Log
	void log(Level p_level, std::u8string_view p_file, uint32_t p_line, std::u8string_view p_message);

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

/// \endcond
