#pragma once

#define SPDLOG_USE_STD_FORMAT
#define SPDLOG_NO_EXCEPTIONS

#ifdef _WIN32
#define SPDLOG_WCHAR_FILENAMES
#endif