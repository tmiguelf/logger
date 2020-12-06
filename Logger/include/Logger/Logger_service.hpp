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

#include "Logger_api.h"
#include "Logger.hpp"
#include "log_sink.hpp"

//======== ======== API ======== ========

using PreLogHandle = void*;

extern "C"
{
	///	\brief	Used by the executable to close the file. make sure all logs are flushed before quiting.
	//
	Logger_API void Log_Shutdown();

	///	\brief Creates a pre-log context handle to allow the user to push messages into pre-log. The handle should not be discarded, and must be destroyed by using \ref Log_DestroyPreLogHandle when no longer needed.
	///	\return A handle to a pre-log context.
	//
	[[nodiscard]] Logger_API PreLogHandle Log_CreatePreLogHandle();

	///	\brief Destroys a pre-log context handle previously created by \ref Log_CreatePreLogHandle.
	///	\param[in] p_handle - A handle to destroy
	//
	Logger_API void Log_DestroyPreLogHandle(PreLogHandle p_handle);

	///	\brief Stores a log message to a pre-log context
	///	\param[in] p_handle			- A handle to a pre-log context previously created by \ref Log_CreatePreLogHandle.
	///	\param[in] p_category		- \ref simLog::LogCategory
	///	\param[in] p_file			- Name of source file generating the log. Please use the Macro __FILE__
	///	\param[in] p_fileNameSize	- Lenght of p_file string
	///	\param[in] p_line			- Source file line number where the Log was generated. Please use the Macro __LINE__
	///	\param[in] p_message		- Message to Log
	///	\param[in] p_messageSize	- Lenght of p_message
	///	\remarks
	///		Logs pushed onto the pre-log context can later be pushed to the log system by using \ref Log_PushPreLog
	//
	Logger_API void Log_2PreLog(PreLogHandle p_handle, logger::Level p_level, const char8_t* p_file, uintptr_t p_fileNameSize, uint32_t p_line, const char8_t* p_message, uintptr_t p_messageSize);

	///	\brief Pushes the logged messages stored on a pre-log context onto the logging system.
	///	\param[in] p_handle - A handle to a pre-log context previously created by \ref Log_CreatePreLogHandle, which contains messages previously stored by \ref Log_2PreLog
	///	\remarks
	///		Should be used after the log system is initialized (\ref Log_PrimeOutputFile). The pre-log context is cleared of all its stored messages, reverting it to a blank state.
	//
	Logger_API void Log_PushPreLog(PreLogHandle p_handle);
} 	//extern "C"


namespace logger
{

Logger_API void Log_add_sink(log_sink& p_stream);
Logger_API void Log_remove_sink(log_sink& p_stream);

//======== ======== Assists ======== ========

///	\brief Stores a log message to a pre-log context
///	\param[in] p_handle - A handle to a pre-log context previously created by \ref Log_CreatePreLogHandle.
///	\param[in] p_category - \ref simLog::LogCategory
///	\param[in] p_file - Name of source file generating the log. Pleas use the Macro __FILE__
///	\param[in] p_line - Source file line number where the Log was generated. Please use the Macro __LINE__
///	\param[in] p_message - Message to Log
///	\remarks
///		Logs pushed onto the pre-log context can later be pushed to the log system by using \ref Log_PushPreLog
//
inline void Log_2PreLog(PreLogHandle p_handle, logger::Level p_level, std::u8string_view p_file, uint32_t p_line, std::u8string_view p_message)
{
	::Log_2PreLog(p_handle, p_level, p_file.data(), p_file.size(), p_line, p_message.data(), p_message.size());
}

}	// namespace simLog
