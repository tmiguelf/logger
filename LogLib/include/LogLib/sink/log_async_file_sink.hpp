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

#include <filesystem>
#include <vector>
#include <queue>
#include <atomic>

#include <CoreLib/core_thread.hpp>
#include <CoreLib/core_sync.hpp>
#include <CoreLib/core_file.hpp>

#include "log_sink.hpp"


namespace logger
{
///	\brief Created to do Logging to file
class log_async_file_sink final: public log_sink
{
public:
	log_async_file_sink();
	~log_async_file_sink();

	///	\brief Logs data to file
	///	\praram[in] - p_logData - Data that will be logged to the file
	void output(const log_data& p_logData) final;

	///	\brief Initiates the logging to File stream,
	///			Creates a file with the given file name
	///	\param[in] - p_fileName - Name of the file that the message will be logged to
	///	\return true on success, false otherwise
	bool init(const std::filesystem::path& p_fileName);

	///	\brief Terminates the logging to File stream,
	///			Closese the file which the message was logged to
	void end();

private:
	void run(void*);
	void dispatch();

	core::file_write m_file; //!< Output file

	std::atomic<bool> m_quit = false;
	core::thread m_thread;
	core::event_trap m_trap;
	core::atomic_spinlock m_lock;
	std::queue<std::vector<char8_t>> m_data;
};

}	// namespace logger
