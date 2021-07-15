Build Status: 
#### CI Suspended until CI tools are updated to the latest copilers

# Logger
Incredibly fast and versatile global logger.

It's simple to setup, but even simpler to use:
 1. Include <Logger.hpp>
 2. Choose a logging category:
	* `LOG_INFO`
	* `LOG_WARNING`
	* `LOG_ERROR`
	* `LOG_DEBUG`
 3. Log your message by listing your arguments in order \
Ex. `LOG_INFO("This is my message. It accepts the usual types ", 42);`


There are 2 types of interfaces:
 * User interface - Generate the log
 * Service management interface - Control what happens to the logs

## Captured datapoints
The following datapoints can be captured by the logger:
 * user defined message - Self explanatory
 * filepath - This can be automatically captured or customized by the user. If automatically captured this will be the source file that generated the log
 * line number - This can be automatically captured or customized by the user. If automatically captured this will be the line in the source file that generated the log
 * column number - This can be automatically captured or customized by the user. If automatically captured this will be 0
 * category - A user specified log category. May also be referred to as "log level".
 * thread id - ID of the thread that generated the log, not customizable by the user
 * Date and time - UTC date a time, not customizable by the user

## User interface
There are 4 main macros that define the basic user interface:
 * `LOG_INFO` - Uses `Info` log category. Intended for low criticality logging, a state in your program may have changed but nothing is unusual.
 * `LOG_WARNING` - Uses `Warning` log category. Intend for medium criticality logging, there's an abnormal condition but the application can confidently recover and continue the task.
 * `LOG_ERROR` - Uses `Error` log category. Intended for high criticality logging, indicates a serious abnormal condition that the application cannot recover, and has no option but to abort the task (either fully or partially).
 * `LOG_DEBUG` - Uses `Debug` log category. Intended for debugging purposes only. In debug builds the message will be logged, in release builds noting will be logged out.

All these 4 macros will automatically capture the file and the line (and the category) in the source code that generated the log.\
The user just needs to lists the content they want to log as arguments.\
Ex. `LOG_WARNING("This is a warning")`

However, if the user whishes too, it is possible to also customize the file and line that they whish to add to the log.
For example, to use in situation where the issue being found is not about an occurrence in the source code, but an occurrence on an external datafile,
and in that situation the user would want to refer to the data file as the source of the problem instead of the source code that issued the warning.\
For this use case the macro `LOG_CUSTOM` is provided. This macro takes in 4 leading parameters like so: \
`LOG_CUSTOM(File, Line, Column, Level)` \
Where:
 * `File` - Is the name of the file. Type must be convertible to `core::os_string_view`. I.e. A `std::basic_string_view` with the type `char` on linux, or type `wchar_t` on windows. (to be compatible with `std::filesystem::path{}.native()`)
 * `Line` - Is the line number. Type `uint32_t`
 * `Column` - Is the column number. Type `uint32_t`
 * `Level`- Is the log category (or log level). An enum of type `logger::Level`

The `LOG_CUSTOM` can also be used to log the same way as the previous macros did, example:\
`LOG_CUSTOM("custom_file_name.txt", 42, 0, logger::Level::Info, "This is my custom message.")`

If instead the user wishes to specify their own custom streamers and take formatting into their own hands, then they can just use the function\
`void Log_Message(Level p_level, core::os_string_view p_file, uint32_t p_line, uint32_t p_column, std::u8string_view p_message)`\
Where:
 * p_level - Is the log level
 * p_file - Is the file name
 * p_line - Is the line number
 * p_column - Is the column number
 * p_message - Is the user message

Regardless of which of the methods used, the "thread id" and "Date and time" are always captured automatically, and cannot be customized (always captured internally).
"Date and time" is always in UTC.

Note: It is not required for the user to provide a new line at the end of the log. By convention a new line is implicit per each call to the log (please see Service management interface for more details).

### Streaming capabilities
The data types that can be passed to the logger are endless.\
Anything that has a defined core::toPrint adapter (from the CoreLib library) can be streamed by default, this includes user defined types.
Or pass any adapter compatible with core::toPrint for any custom format.

## Service management interface
What happens to a generated log once dispatched can be controlled by the management interface.
This interface can be accessed by including the header `Logger_service.hpp`.

At the start of a program, nothing happens to any dispatched logs. This is because there's nowhere that has been defined for the logs to go.
To define where the logs are forward to, you must first register a sink that receives those logs and decides how to dispatch them.
A sink for example can decide to write the Logs to a file, or print it on the console, or forward it to a socket, or anything that the user wishes to implement.

The following functions are available:
 * `Log_add_sink` - Registers a sink. The life-time of the sink must be guaranteed until it's unregistered.
 * `Log_remove_sink` - Unregister a specific sink.
 * `Log_remove_all` - Unregisters all sinks.

It is possible to register multiple sinks, in this case a generated log is forward to all sinks sequentially by order of registration.

### Provided sinks
The following sinks are provided with this library:
 * logger::log_file_sink - Used to log to a file. Defined in header `log_file_sink.hpp`.
 * logger::log_console_sink - Used to log to `std::cout`. Defined in header `log_console_sink.hpp`.

The user can create their own custom sink by inheriting from `logger::log_sink` defined in header `log_sink.hpp`. Note that by convention, the user need not specify a new line at the end of a message (implicit), and thus one will not exist at the end of the message. The implementer of the sink should honor this agreement by adding any extra new line at the end of the stream (if applicable).

### Windows only
On a windows only, this library provides a sink that can send the logs to the debugger console (for example Visual Studio console).
In Visual Studio, this supports the functionality to be able to jump to the referenced file and line when double clicking on the logged message.

## Thread safety
Logging is as thread as the `output` method of the sinks. (I.e. If the `output` is thread safe, logging is thread safe).\
As a convention, users trying to generate logs should not have to worry about thread safety, and it is thus recommended for sink designers to ensure that their sinks are thread safe.

Registering and unregistering sinks is not thread safe, do not attempt to register or unregister sinks simultaneously in different threads, or try to log while registering/unregistering sinks.
These will lead to a race condition and cause undefined behavior.
