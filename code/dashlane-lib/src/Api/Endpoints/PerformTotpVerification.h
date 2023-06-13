#pragma once

#include <Api/ApiRequest.h>

namespace Dashlane
{

	inline EDashlaneError PerformTotpVerification(DashlaneContextInternal& context, std::string& response)
	{
		nlohmann::ordered_json result;
		EDashlaneError rc = Dashlane::RequestApi(
			context,
			"authentication/PerformTotpVerification",
			result,
			{
				{"login", context.login},
				{"otp", context.secrets.twoFactorCode},
				{"activationFlow", false }
			}
		);

		if (rc == EDashlaneError::NoError)
		{
			response = result.at("data").at("authTicket").get<std::string>();
		}

		return rc;
	}

}