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

#include "Logger/log_file_sink.hpp"
#include "Logger/log_streamer.hpp"

#include <CoreLib/string/core_string_encoding.hpp>

namespace logger
{

inline static void AuxWrite(std::basic_ostream<char8_t>& p_strm, std::u8string_view p_out)
{
	p_strm.write(p_out.data(), p_out.size());
}

log_file_sink::log_file_sink() = default;

log_file_sink::~log_file_sink()
{
	end();
}

void log_file_sink::output(const log_data& p_logData)
{
	_p::u8string_stream ts;

	ts	<< p_logData.m_dateTimeThread
#ifdef _WIN32
		<< core::UTF16_to_UTF8_faulty(std::u16string_view{reinterpret_cast<const char16_t*>(p_logData.m_file.data()), p_logData.m_file.size()}, '?')
#else
		<< reinterpret_cast<const std::u8string_view&>(p_logData.m_file)
#endif
		<< u8'('
		<< p_logData.m_line;

	if(p_logData.m_columnNumber)
	{
		ts << u8',' << p_logData.m_column;
	}

	ts	<< u8") "
		<< p_logData.m_level
		<< p_logData.m_message
		<< '\n';

	//AuxWrite(m_output, ts.view());
	AuxWrite(m_output, ts.str());
}

bool log_file_sink::init(const std::filesystem::path& p_fileName)
{
	end();
	bool input_absolute = p_fileName.is_absolute();
	std::error_code ec;
	const std::filesystem::path& fileName =
		input_absolute ?
		p_fileName.parent_path() :
		std::filesystem::absolute(p_fileName, ec);

	if(!input_absolute && ec != std::error_code{})
	{
		return false;
	}

	std::filesystem::create_directories(fileName.parent_path());
	m_output.open(p_fileName, std::ios_base::binary | std::ios_base::out);
	return m_output.is_open();
}

void log_file_sink::end()
{
	m_output << std::flush;
	m_output.close();
}

} //namespace simLog
