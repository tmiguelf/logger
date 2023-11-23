//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///
///	\copyright
///		Copyright (c) Tiago Miguel Oliveira Freire
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

#include <Logger/sink/log_async_file_sink.hpp>

#include <array>
#include <vector>
#include <utility>

#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/Core_Alloca.hpp>

namespace logger
{

static inline void transfer(char8_t*& p_buff, const std::u8string_view p_str)
{
	memcpy(p_buff, p_str.data(), p_str.size());
	p_buff += p_str.size();
}


log_async_file_sink::log_async_file_sink() = default;

log_async_file_sink::~log_async_file_sink()
{
	end();
}


void log_async_file_sink::output(const log_data& p_logData)
{
	if(!m_file.is_open()) return;

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

	std::vector<char8_t> buff;
	buff.resize(count);

	{
		char8_t* pivot = buff.data();
		*(pivot++) = u8'[';
		transfer(pivot, p_logData.m_date);
		*(pivot++) = u8'-';
		transfer(pivot, p_logData.m_time);
		*(pivot++) = u8'|';
		transfer(pivot, p_logData.m_thread);
		*(pivot++) = u8']';

#ifdef _WIN32
		core::_p::UTF16_to_UTF8_faulty_unsafe(std::u16string_view{reinterpret_cast<const char16_t*>(p_logData.m_file.data()), p_logData.m_file.size()}, '?', pivot);
		pivot += fileSize_estimate;
#else
		memcpy(pivot, p_logData.m_file.data(), p_logData.m_file.size());
		pivot += p_logData.m_file.size();
#endif

		* (pivot++) = u8'(';
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
	}

	{
		const core::atomic_spinlock::scope_locker lock{m_lock};
		m_data.emplace(std::move(buff));
	}
	m_trap.signal();
}

bool log_async_file_sink::init(const std::filesystem::path& p_fileName)
{
	end();
	const bool input_absolute = p_fileName.is_absolute();
	std::error_code ec;
	const std::filesystem::path& fileName =
		input_absolute ?
		p_fileName :
		std::filesystem::absolute(p_fileName, ec);

	if(!input_absolute && ec != std::error_code{})
	{
		return false;
	}

	if(m_file.open(fileName, core::file_write::open_mode::create, true) != std::errc{})
	{
		return false;
	}

	m_quit.store(false, std::memory_order::relaxed);
	m_trap.reset();
	if(m_thread.create(this, &log_async_file_sink::run, nullptr) != core::thread::Error::None)
	{
		m_file.close();
		return false;
	}

	return true;
}

void log_async_file_sink::end()
{
	if(m_thread.joinable())
	{
		m_quit.store(true, std::memory_order::release);
		m_trap.signal();
		m_thread.join();
	}

	m_file.flush();
	m_file.close();
}

void log_async_file_sink::run(void*const)
{
	constexpr std::array UTF8_BOM = {char8_t{0xEF}, char8_t{0xBB}, char8_t{0xBF}};

	m_file.write_unlocked(UTF8_BOM.data(), UTF8_BOM.size());

	while(!m_quit.load(std::memory_order::acquire))
	{
		m_trap.wait();
		m_trap.reset();
		while(!m_data.empty())
		{
			dispatch();
		}
	}
	dispatch();
}

void log_async_file_sink::dispatch()
{
	std::queue<std::vector<char8_t>> local;
	{
		core::atomic_spinlock::scope_locker lock{m_lock};
		m_data.swap(local);
	}

	while(!local.empty())
	{
		std::vector<char8_t>& obj = local.front();
		m_file.write_unlocked(obj.data(), obj.size());
		local.pop();
	}
}

} //namespace simLog
