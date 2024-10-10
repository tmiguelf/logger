Play video>

[![IMAGE ALT TEXT HERE](http://img.youtube.com/vi/6v1uG4JJ59Q/0.jpg)](http://www.youtube.com/watch?v=6v1uG4JJ59Q)


Build Status: 
#### CI Suspended until CI tools are updated to the latest compilers

# Logger
The fastest* logging library of its kind, and an extremely versatile global logger.

It's simple to setup, but even simpler to use:
 1. Include <Logger.hpp>
 2. Choose a logging category:
	* `LOG_INFO`
	* `LOG_WARNING`
	* `LOG_ERROR`
	* `LOG_DEBUG`
 3. Log your message by listing your arguments in order \
Ex. `LOG_INFO("This is my message. It accepts the usual types "sv, 42);`


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
 * base address - the base address of the module generating the log, not intended for user customization

## User interface
There are 4 main macros that define the basic user interface:
 * `LOG_INFO()` - Uses `Info` log category. Intended for low criticality logging, a state in your program may have changed but nothing is unusual.
 * `LOG_WARNING()` - Uses `Warning` log category. Intend for medium criticality logging, there's an abnormal condition but the application can confidently recover and continue the task.
 * `LOG_ERROR()` - Uses `Error` log category. Intended for high criticality logging, indicates a serious abnormal condition that the application cannot recover, and has no option but to abort the task (either fully or partially).
 * `LOG_DEBUG()` - Uses `Debug` log category. Intended for debugging purposes only. In debug builds the message will be logged, in release builds noting will be logged out.

All these 4 macros will automatically capture the file and the line (and the category) in the source code that generated the log.\
The user just needs to lists the content they want to log as arguments.\
Ex. `LOG_WARNING("This is a warning"sv)`

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
`LOG_CUSTOM("custom_file_name.txt", 42, 0, logger::Level::Info, "This is my custom message."sv)`

Regardless of which of the methods used, the "thread id" and "Date and time" are always captured automatically, and cannot be customized (always captured internally).
The generating module base address is intended to be automatically captured, but is reliant on fudgeable client side data hacking.
"Date and time" are always in UTC.

Note: It is not required for the user to provide a new line at the end of the log. By convention a new line is implicit per each call to the log (please see Service management interface for more details).

### Streaming capabilities
The data types that can be passed to the logger are endless.\
Anything that has a defined core::toPrint adapter (from the CoreLib library) can be streamed by default without any extra enhancements, this includes user defined types.
Or you can pass any adapter compatible with core::toPrint for any custom format.

## Service management interface
### Sinks
What happens to a generated log once it's dispatched can be controlled by the management interface.
This interface can be accessed by including the header `Logger_service.hpp`.

At the start of a program, nothing happens to any dispatched logs. This is because there's nowhere that has been defined for the logs to go.
To define where the logs are forward to, you must first register a sink that receives those logs and decides how to dispatch them.
A sink for example can decide to write the Logs to a file, or print it on the console, or forward it to a socket, or anything that the user wishes to implement.

The following functions are available:
 * `log_add_sink` - Registers a sink. The life-time of the sink must be guaranteed until it's unregistered.
 * `log_remove_sink` - Unregister a specific sink.
 * `log_remove_all` - Unregisters all sinks.

It is possible to register multiple sinks, in this case a generated log is forward to all sinks sequentially by order of registration.

#### Provided sinks
The following sinks are provided with this library:
 * logger::log_file_sink - Used to log to a file. Defined in header `log_file_sink.hpp`.
 * logger::log_console_sink - Used to log to `std::cout`. Defined in header `log_console_sink.hpp`.

The user can create their own custom sink by inheriting from `logger::log_sink` defined in header `log_sink.hpp`. Note that by convention, the user need not specify a new line at the end of a message (implicit), and thus one will not exist at the end of the message. The implementer of the sink should honor this agreement by adding any extra new line at the end of the stream (if applicable).

#### Windows only
On a windows only, this library provides a sink that can send the logs to the debugger console (for example Visual Studio console).
In Visual Studio, this supports the functionality to be able to jump to the referenced file and line when double clicking on the logged message.

### Filter
In addition this library supports global filtering, i.e. the user can specify its own custom filter to pick exactly which logs to process.
By default, if no user filter is registered all logs are accepted.
The folloing function are available:
 * `log_set_filter` - Sets a custom filter. The life-time of the filter must be guaranteed until it's unregistered.
 * `log_reset_filter` - Resets the filter to the default filter and set its behaviour (i.e. either accept all or reject all)

A user may write its own filter by inheriting from the abstract class `log_filter` and defining the behaviour of the `filter` virtual method, if `filter` returns true the log is accepted.
Note: The filter will allways receive the "file" and "line" of the corresponding source code generating the log, even if the user specified a custom "file" and "line" when using LOG_CUSTOM,
this is so that developers are able to effectly write filters targeting specific components in their applications without being blinded by content that maybe runtime specific.

## Thread safety
Logging is as thread as the `output` method of the sinks. (I.e. If the `output` is thread safe, logging is thread safe).\
As a convention, users trying to generate logs should not have to worry about thread safety, and it is thus recommended for sink designers to ensure that their sinks are thread safe.

Registering and unregistering sinks/filters is not thread safe, do not attempt to register or unregister sinks/filters simultaneously in different threads, or try to log while registering/unregistering sinks/filters.
These will lead to a race condition and cause undefined behavior.

## Benchmarking
I have added benchmarks against the following popular libraries (considered fast):
 * [spdlog](https://github.com/gabime/spdlog)
 * [g3log](https://github.com/KjellKod/g3log)
 * [NanoLog](https://github.com/Iyengar111/NanoLog)

Feel free to send those projects a thumbs up.
There were other libraries that were considered, however they had been disqualified out of the fact that they wouldn't compile at all.
And as harsh as it sounds, if the code doesn't so much as compile, you don't really have much of a working product (logger) as much as you have a randomly collected
set of ideas. It's just hypothetical.

Before we show any benchmarks, we need to establish some common ground and make sure we are comparing apples to apples.\
However, exact common ground isn't always possible (and does not occur in this case).\
Different libaries have been optimized for different use cases, and it is possible to design benchmarks with particular uses cases that would penalize one over the other.

Is the ability to have multiple non-file sinks important?

What is the size in bytes of the generated meta-data?\
Larger maybe more costly, but what if the extra information is crucial for your system?

Does the library provide date-time pre-formatting capabilities?\
If you have multiple sinks, pre-formatting would make it faster than have each individual sink formatting it.\
However, if you don't care for date and time in your logs, pre-formatting would actually cost you.

Do you expect large or small number of concurrent threads writing to the log? \
There are synchronization mechanisms that are more costly with a lower number of threads but scale better with a larger number of threads.

When you log to a file, does the your system bottleneck on "memory allocation"/"concurrency management" or on disk access?\
If your system bottlenecks on disk, delegating the write to a separate thread would make it faster.\
However, if it bottlenecks on "memory allocation"/"concurrency management", writing the output directly onto disk can be much faster than delegating it to a separate thread.

Each takes different solutions to address these problems. So take any benchmarked values (including these ones) with a grain of salt. \
Make sure to do your own research first, and pick the one that best fits your usage scenario. \
If this particular library isn't the best for what you are doing, then that's ok, please consider taking a look at others and check if their strengths better align with your application.


### Benchmarks
There are 2 types of benchmarks provided, but each try to measure one thing, "return time", i.e. The time it takes in the critical path of the user code between the point where a decision to generate a Log is made and the Log call returns the execution to the user's code.
What it tries to measure is the performance impact a logging call has, lower times are better.\
Note: We are not measuring how much time it takes for a log to be available to read on disk, or be available for read at whatever custom sink the user cares about.
Asynchronous loggers can delegate the write task to a separate thread, and take longer to write on disk than others, but can appear much faster because they return execution to the user sooner.

 1. vs_benchmark - Only for those who support custom sinks. For this a custom output function is provided who’s only tasks is to prevent code form being optimized away.
    Each logger must create a custom sink that pushes the logged message onto this function. This will serve as a placeholder for any custom sink the user will be able to provide.
    The default implementation of each type of sink, that is provided by default with these libraries, maybe extremely unperforming, and thus have a huge impact on their times,
    but theoretically because a user can write their own sinks, the performance can theoretically be made much faster. This test aims at removing the impact of the sinks in the tests results.
    NanoLog is excluded from this as it does not support custom sinks.

    In this test there are 3 subtests:\
      a) Log a message - To measure the general capability of just forwarding a simple message at all.\
      b) Log nothing - Logging nothing isn't really just logging nothing, meta-data is still captured and may end up in the logs. Useful in situations where the meta-data alone is enough to understand the issue being logged. Measures the penalty of meta-data collection.\
      c) Log a combination of text and numbers - 1 string_view, 1 int, 1 unsigned int, 1 double, and 1 lonely character. Helps measure the performance of the formatting, similar to the general use case where logs have dynamic information in them.

      [benchmark](https://github.com/google/benchmark) is used for these.

 2. disk_bench - This aims to make a more realistic impact measurement with each library's "file sinks" in the loop.
    The same formatting input as 1.c (in vs_benchmark) is used with the significant difference that the numeric values are rolling as the tests progresses.
    5 different threads running concurrently each trying to log 100000 times.
On this test there are 2 notable discrepancies:\
      a) The logger tests 2 types of file sinks, synchronous and asynchronous.\
      b) spdlog uses the single-threaded (st) version of the file sink, instead of the multi-threaded (mt).
         This was done because their st version actually appears to be thread safe, and because it doesn't use any mutexes, it results in a much faster code.
         Although the mt version was the one intended to be used in such scenario, someone with sufficient powers of observation could come to the same conclusion
         and opt to use the st version instead. We want to give these libraries their best chance of succeeding.

Here are the results:

vs_benchmark:
MSVC Windows:
| nano seconds | Logger | spdlog | spdlog +time | g3log |
| ------------ | ------ | ------ | ------------ | ----- |
| Combination  |    118 |    221 |          244 |  1595 |
| String       |   65.1 |   28.5 |         44.7 |   750 |
| Nothing      |   63.5 |   45.0 |         60.5 |   579 |

Gcc Linux:
| nano seconds | Logger | spdlog | spdlog +time | g3log |
| ------------ | ------ | ------ | ------------ | ----- |
| Combination  |   67.1 |    162 |          174 |   913 |
| String       |   45.4 |   36.1 |         53.0 |   747 |
| Nothing      |   45.0 |   38.3 |         60.0 |   517 |

disk_bench (Windows):
| Library      | Seconds |
| ------------ | ------- |
| Logger       |  0.1537 |
| Logger Async |  0.0892 |
| spdlog       |  0.1993 |
| g3log        |  1.4368 |
| NanoLog      |  2.3074 |

On the vs_benchmark, Logger wins when there's formatting involved, but loses to spdlog when there's just a string or there's nothing to log.
This is due to the fact that Logger has a more complete data capture and pre-formatting (the cost of that alone is about 30ns on windows ouch!),
spdlog doesn't collect time-stamps by default and would need to incur that extra cost later if the sink wished to log that data.
spdlog would be slower if comparable data were to be formatted onto an output stream.
However, Logger has a much more efficient formatting library, so it ends up ahead when formatting is involved.

On the disk_bench, Logger is the winner with NanoLog the clear loser. Logger and spdlog perform very closely since they are both being bottlenecked
by the synchronous write to disk (splog generates shorter messages), and they both work pretty much in a similar fashion (fwrite).
Logger in asynchronous mode comes up ahead, but I believe spdlog could achieve similar results if an asynchronous sink was added.


Q: Why weren't libraries such as [Glog](https://github.com/google/glog) or [reckless](https://github.com/mattiasflodin/reckless) included in the benchmark.\
A: All of the external libraries in this benchmark had bugs in them (obvious ones) that prevented them from being compiled with my setup.
The ones that eventually made it onto the board were simple enough to fix.
I have attempted to benchmark Glog and reckless, however there were way too many bugs in them to be able to compile with even the most laxed of rules.
Sorry for the developers of these libraries, it was way too much work for me to fix them, if you managed to correct them in the future, I will consider adding them.
