#pragma once

#include <string>

#ifndef dashlane_lib_STATIC
	#ifdef dashlane_lib_EXPORTS
		#define DASHLANE_API __declspec(dllexport)
	#else
		#define DASHLANE_API __declspec(dllimport)
	#endif
#else
	#define DASHLANE_API
#endif