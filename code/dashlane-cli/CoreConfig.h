#pragma once

#ifdef DCCLI_ACCESS_KEYS
#include "AccessKeys.h"
static_assert(sizeof(szAppAccessKey) > 1, "Application Access Key is not defined");
static_assert(sizeof(szAppSecretKey) > 1, "Application Secret Key is not defined");
#else
#error You must provide API Access Keys via _private/AccessKeys.h. Please refer to documentation.
#endif

namespace Dashlane
{

	static constexpr char applicationName[] = "Dashlane CLI v1.0";

}