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

#include <Logger/sink/log_file_sink.hpp>

#include <array>
#include <vector>

#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/Core_Alloca.hpp>

namespace logger
{

static inline void transfer(char8_t*& p_buff, std::u8string_view p_str)
{
	memcpy(p_buff, p_str.data(), p_str.size());
	p_buff += p_str.size();
}

#ifdef _WIN32
#	define AUX_WRITE_DATA(STREAM, DATA, BUFFER, SIZE, FILE_SIZE) AuxWriteData(STREAM, DATA, BUFFER, SIZE, FILE_SIZE)
#else
#	define AUX_WRITE_DATA(STREAM, DATA, BUFFER, SIZE, FILE_SIZE) AuxWriteData(STREAM, DATA, BUFFER, SIZE)
#endif

static inline void
	AUX_WRITE_DATA(std::basic_ostream<char8_t>& p_strm,
		const log_data& p_logData,
		char8_t* const p_buffer,
		uintptr_t p_buffer_size,
		uintptr_t p_fileName_size)
{
	char8_t* pivot = p_buffer;
	*(pivot++) = u8'[';
	transfer(pivot, p_logData.m_date);
	*(pivot++) = u8'-';
	transfer(pivot, p_logData.m_time);
	*(pivot++) = u8'|';
	transfer(pivot, p_logData.m_thread);
	*(pivot++) = u8']';

#ifdef _WIN32
	core::_p::UTF16_to_UTF8_faulty_unsafe(std::u16string_view{reinterpret_cast<const char16_t*>(p_logData.m_file.data()), p_logData.m_file.size()}, '?', pivot);
	pivot += p_fileName_size;
#else
	memcpy(pivot, p_logData.m_file.data(), p_logData.m_file.size());
	pivot += p_logData.m_file.size();
#endif

	*(pivot++) = u8'(';
	transfer(pivot, p_logData.m_line);

	if(p_logData.m_columnNumber)
	{
		*(pivot++) = u8',';
		transfer(pivot, p_logData.m_column);
	}
	*(pivot++) = u8')';
	*(pivot++) = u8' ';
	transfer(pivot, p_logData.m_level);
	transfer(pivot, p_logData.m_message);
	*(pivot) = u8'\n';
	p_strm.write(p_buffer, p_buffer_size);
}



log_file_sink::log_file_sink() = default;

log_file_sink::~log_file_sink()
{
	end();
}


void log_file_sink::output(const log_data& p_logData)
{
#ifdef _WIN32
	const uintptr_t fileSize_estimate = core::_p::UTF16_to_UTF8_faulty_estimate(std::u16string_view{reinterpret_cast<const char16_t*>(p_logData.m_file.data()), p_logData.m_file.size()}, '?');
#else
	const uintptr_t fileSize_estimate = p_logData.m_file.size();
#endif

	//[date]File(Line,Column) Message\n
	const uintptr_t count =
		p_logData.m_date.size()
		+ p_logData.m_time.size()
		+ p_logData.m_thread.size()
		+ fileSize_estimate
		+ p_logData.m_line.size()
		+ (p_logData.m_columnNumber ? p_logData.m_column.size() + 1 : 0) //,
		+ p_logData.m_level.size()
		+ p_logData.m_message.size() + 8; //[-|]() \n

	constexpr uintptr_t alloca_treshold = 0x10000;

	if(count > alloca_treshold)
	{
		std::vector<char8_t> buff;
		buff.resize(count);
		AUX_WRITE_DATA(m_output, p_logData, buff.data(), count, fileSize_estimate);
	}
	else
	{
		char8_t* buff = reinterpret_cast<char8_t*>(core_alloca(count));
		AUX_WRITE_DATA(m_output, p_logData, buff, count, fileSize_estimate);
	}
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
	if(!m_output.is_open()) return false;

	constexpr std::array UTF8_BOM = {char8_t{0xEF}, char8_t{0xBB}, char8_t{0xBF}};
	m_output.write(UTF8_BOM.data(), UTF8_BOM.size());

	return true;
}

void log_file_sink::end()
{
	m_output << std::flush;
	m_output.close();
}

} //namespace simLog
