#pragma once

#include <Api/ApiRequest.h>

namespace Dashlane
{

	inline EDashlaneError PerformDuoPushVerification(DashlaneContextInternal& context, std::string& response)
	{
		nlohmann::ordered_json result;
		EDashlaneError rc = Dashlane::RequestApi(
			context,
			"authentication/PerformDuoPushVerification",
			result,
			{
				{"login", context.login}
			}
		);

		if (rc == EDashlaneError::NoError)
		{
			response = result.at("data").at("authTicket").get<std::string>();
		}

		return rc;
	}

}