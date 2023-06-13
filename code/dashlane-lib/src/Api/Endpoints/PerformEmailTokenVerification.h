#pragma once

#include <Api/ApiRequest.h>

namespace Dashlane
{

	inline EDashlaneError PerformEmailTokenVerification(DashlaneContextInternal& context, std::string& response)
	{
		nlohmann::ordered_json result;
		EDashlaneError rc = Dashlane::RequestApi(
			context,
			"authentication/PerformEmailTokenVerification",
			result,
			{
				{"login", context.login},
				{"token", context.secrets.emailToken}
			}
		);

		if (rc == EDashlaneError::NoError)
		{
			response = result.at("data").at("authTicket").get<std::string>();
		}

		return rc;
	}

}