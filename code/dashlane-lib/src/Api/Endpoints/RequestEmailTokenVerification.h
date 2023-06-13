#pragma once

#include <Api/ApiRequest.h>

namespace Dashlane
{

	inline EDashlaneError RequestEmailTokenVerification(DashlaneContextInternal& context)
	{
		nlohmann::ordered_json result;
		EDashlaneError rc = Dashlane::RequestApi(
			context,
			"authentication/RequestEmailTokenVerification",
			result,
			{
				{"login", context.login}
			}
		);
		return rc;
	}

}