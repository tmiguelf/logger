//======== ======== ======== ======== ======== ======== ======== ========
///	\file
///		Export definition for Logger
///
///	\copyright
///		
//======== ======== ======== ======== ======== ======== ======== ========

#pragma once

#include <extension/dll_api_macros.h>

#ifdef _Logger_EXPORTS_
#	define Logger_API DLL_EXPORT
#else
#	define Logger_API DLL_IMPORT
#endif // _Logger_EXPORTS_
