#pragma once

#include <dashlane/API.h>
#include <dashlane/Errors.h>

#include "Utility/ConceptHelpers.h"
#include "Utility/EnumClass.h"

#include <base64pp/base64pp.h>
#include <nlohmann/json.hpp>

#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#include <filesystem>

#ifdef _DEBUG
#include <iostream>
#endif

#if defined(WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <Shlwapi.h>
#include <shlobj_core.h>
#include <io.h> 
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#endif

#ifdef MACOS
#include <libgen.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <unistd.h>
#endif

#ifdef LINUX
#include <limits.h>
#include <libgen.h>
#include <unistd.h>
#if defined(__sun)
#define PROC_SELF_EXE "/proc/self/path/a.out"
#else
#define PROC_SELF_EXE "/proc/self/exe"
#endif
#endif