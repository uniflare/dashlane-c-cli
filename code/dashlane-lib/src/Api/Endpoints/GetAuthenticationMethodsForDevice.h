#pragma once

#include <Dashlane.h>
#include <Api/ApiRequest.h>

namespace Dashlane
{

	static std::vector<EAuthMethod> s_defaultSupportedMethods = 
	{
		EAuthMethod::EmailToken,
		EAuthMethod::Totp,
		EAuthMethod::DuoPush,
		EAuthMethod::DashlaneAuthenticator
	};

	inline void from_json(const nlohmann::ordered_json& j, SAuthenticationMethodU2F::SChallenge& p)
	{
		j.at("challenge").get_to(p.challenge);
		j.at("version").get_to(p.version);
		j.at("appId").get_to(p.appId);
		j.at("keyHandle").get_to(p.keyHandle);
	}

    inline void from_json(const nlohmann::ordered_json& j, std::unique_ptr<IAuthenticationMethodBase>& p)
    {
        switch (j.at("type").get<EAuthMethod>())
        {

        case EAuthMethod::SSO:
        {
            p = std::make_unique<SAuthenticationMethodSSO>();
            auto& obj = static_cast<SAuthenticationMethodSSO&>(*p);
            j.at("ssoInfo").at("serviceProviderUrl").get_to(obj.serviceProviderUrl);
            j.at("ssoInfo").at("migration").get_to(obj.migration);
            j.at("ssoInfo").at("isNitroProvider").get_to(obj.isNitroProvider);
        } break;

        case EAuthMethod::U2F:
        {
            p = std::make_unique<SAuthenticationMethodU2F>();
            auto& obj = static_cast<SAuthenticationMethodU2F&>(*p);
            j.at("challenges").get_to(obj.challenges);
        } break;

        case EAuthMethod::DashlaneAuthenticator:
        {
            p = std::make_unique<SAuthenticationMethodDashlaneAuthenticator>();
        } break;

        case EAuthMethod::DuoPush:
        {
            p = std::make_unique<SAuthenticationMethodDuoPush>();
        } break;

        case EAuthMethod::EmailToken:
        {
            p = std::make_unique<SAuthenticationMethodEmailToken>();
        } break;

        case EAuthMethod::Totp:
        {
            p = std::make_unique<SAuthenticationMethodTotp>();
        } break;

        }
	}

	inline EDashlaneError GetAuthenticationMethodsForDevice(DashlaneContextInternal& context,
		std::vector<std::unique_ptr<Dashlane::IAuthenticationMethodBase>>& possibleMethods
	)
	{
		nlohmann::ordered_json response;
		EDashlaneError rc = Dashlane::RequestApi(
			context,
			"authentication/GetAuthenticationMethodsForDevice",
			response,
			{
				{"login", context.login},
				{"methods", Dashlane::s_defaultSupportedMethods}
			}
		);

		if (rc == EDashlaneError::NoError)
			possibleMethods = response.at("data").at("verifications").get<std::vector<std::unique_ptr<Dashlane::IAuthenticationMethodBase>>>();

		return rc;
	}

}