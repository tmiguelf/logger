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

#include <fstream>
#include <filesystem>

#include "log_sink.hpp"
#include "Logger_api.h"

namespace logger
{
///	\brief Created to do Logging to file
class log_file_sink final: public log_sink
{
public:
	Logger_API log_file_sink();
	Logger_API ~log_file_sink();

	///	\brief Logs data to file
	///	\praram[in] - p_logData - Data that will be logged to the file
	void output(const log_data& p_logData) final;

	///	\brief Initiates the logging to File stream,
	///			Creates a file with the given file name
	///	\param[in] - p_fileName - Name of the file that the message will be logged to
	///	\return true on success, false otherwise
	Logger_API bool init(const std::filesystem::path& p_fileName);

	///	\brief Terminates the logging to File stream,
	///			Closese the file which the message was logged to
	Logger_API void end();

private:
	std::basic_ofstream<char8_t> m_output; //!< Output file
};

}	// namespace logger
