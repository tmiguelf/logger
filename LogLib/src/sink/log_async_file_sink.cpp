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

#include <LogLib/sink/log_async_file_sink.hpp>

#include <array>
#include <vector>
#include <utility>

#include <CoreLib/string/core_string_encoding.hpp>
#include <CoreLib/core_alloca.hpp>

namespace logger
{

static inline void transfer(char8_t*& p_buff, std::u8string_view const p_str)
{
	memcpy(p_buff, p_str.data(), p_str.size());
	p_buff += p_str.size();
}


log_async_file_sink::log_async_file_sink() = default;

log_async_file_sink::~log_async_file_sink()
{
	end();
}


void log_async_file_sink::output(log_data const& p_logData)
{
	if(!m_file.is_open()) return;

#ifdef _WIN32
	uintptr_t const fileSize_estimate = core::UTF16_to_UTF8_faulty_size(std::u16string_view{reinterpret_cast<char16_t const*>(p_logData.file.data()), p_logData.file.size()}, '?');
#else
	uintptr_t const fileSize_estimate = p_logData.file.size();
#endif

	//[date]File(Line,Column) Message\n
	uintptr_t const count =
		p_logData.sv_date.size()
		+ p_logData.sv_time.size()
		+ p_logData.sv_thread.size()
		+ fileSize_estimate
		+ p_logData.sv_line.size()
		+ (p_logData.column ? p_logData.sv_column.size() + 1 : 0) //,
		+ p_logData.sv_level.size()
		+ p_logData.message.size() + 10; //[-|]() : \n

	std::vector<char8_t> buff;
	buff.resize(count);

	{
		char8_t* pivot = buff.data();
		*(pivot++) = u8'[';
		transfer(pivot, p_logData.sv_date);
		*(pivot++) = u8'-';
		transfer(pivot, p_logData.sv_time);
		*(pivot++) = u8'|';
		transfer(pivot, p_logData.sv_thread);
		*(pivot++) = u8']';

#ifdef _WIN32
		core::UTF16_to_UTF8_faulty_unsafe(std::u16string_view{reinterpret_cast<char16_t const*>(p_logData.file.data()), p_logData.file.size()}, '?', pivot);
		pivot += fileSize_estimate;
#else
		memcpy(pivot, p_logData.file.data(), p_logData.file.size());
		pivot += p_logData.file.size();
#endif

		*(pivot++) = u8'(';
		transfer(pivot, p_logData.sv_line);

		if(p_logData.column)
		{
			*(pivot++) = u8',';
			transfer(pivot, p_logData.sv_column);
		}
		*(pivot++) = u8')';
		*(pivot++) = u8' ';
		transfer(pivot, p_logData.sv_level);
		*(pivot++) = u8':';
		*(pivot++) = u8' ';
		transfer(pivot, p_logData.message);
		*(pivot) = u8'\n';
	}

	{
		core::atomic_spinlock::scope_locker const lock{m_lock};
		m_data.emplace(std::move(buff));
	}
	m_trap.signal();
}

bool log_async_file_sink::init(std::filesystem::path const& p_fileName)
{
	end();
	bool const input_absolute = p_fileName.is_absolute();
	std::error_code ec;
	std::filesystem::path const& fileName =
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
