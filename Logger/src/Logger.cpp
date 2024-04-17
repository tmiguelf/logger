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

#include <CoreLib/core_time.hpp>
#include <CoreLib/core_thread.hpp>
#include <CoreLib/string/core_os_string.hpp>
#include <CoreLib/string/core_string_numeric.hpp>

#include <LogLib/logger_struct.hpp>
#include <LogLib/log_filter.hpp>
#include <LogLib/logger_group.hpp>

#include <Logger/Logger_client.hpp>
#include <Logger/Logger_service.hpp>


//Right now we are enforcing validity by having buffer larger than what we would theorethically need
static constexpr uintptr_t g_DateMessageSize = sizeof("00000/00/00") - 1;
static constexpr uintptr_t g_TimeMessageSize = sizeof("00:00:00.000") - 1;

/// \n
namespace logger
{

//======== ======== ======== ======== Global ======== ======== ======== ========
static LoggerGroup g_logger; //!< Preserves settings
static log_filter const* g_filter = nullptr;
static bool g_default_filter_behaviour = true;

//======== ======== ======== ======== Public API ======== ======== ======== ========

Logger_API void log_add_sink(log_sink& p_stream)
{
	g_logger.add_sink(p_stream);
}

Logger_API void log_remove_sink(log_sink& p_stream)
{
	g_logger.remove_sink(p_stream);
}

Logger_API void log_remove_all()
{
	g_logger.clear();
}

Logger_API void log_message(log_message_data const& data, std::u8string_view message)
{
	g_logger.log(data, message);
}

Logger_API void log_set_filter(log_filter const& p_filter)
{
	g_filter = &p_filter;
}

Logger_API void log_reset_filter(bool p_default_behaviour)
{
	g_filter = nullptr;
	g_default_filter_behaviour = p_default_behaviour;
}

namespace _p
{

Logger_API bool log_check_filter(log_filter_data const& p_data)
{
	if(g_filter)
	{
		return g_filter->filter(p_data);
	}
	return g_default_filter_behaviour;
}

} //namespace _p

}// namespace logger
