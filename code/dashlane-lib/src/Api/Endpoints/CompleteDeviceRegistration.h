#pragma once

#include <Dashlane.h>
#include <Api/ApiRequest.h>

namespace Dashlane
{

	struct SDeviceRegistrationResponse
	{
		struct SRemoteKey
		{
			std::string uuid;
			std::string key;
			EAuthTicketType type{ EAuthTicketType::MASTER_PASSWORD };
		};

		std::string deviceAccessKey;
		std::string deviceSecretKey;

		struct
		{
			size_t backupDate{ 0 };
			std::string identifier;
			size_t time{ 0 };
			std::string content;
			std::string type = "SETTINGS";
			std::string action = "BACKUP_EDIT";
		} settings;

		struct
		{
			std::string privateKey;
			std::string publicKey;
		} sharingKeys;

		std::vector<SRemoteKey> remoteKeys;

		size_t numberOfDevices{ 0 };
		bool hasDesktopDevices{ false };
		std::string publicUserId;
		std::string userAnalyticsId;
		std::string deviceAnalyticsId;
		std::string ssoServerKey;
		std::string serverKey;
	};

    inline void from_json(const nlohmann::ordered_json& j, SDeviceRegistrationResponse::SRemoteKey& p)
    {
        j.at("uuid").get_to(p.uuid);
        j.at("key").get_to(p.key);
        j.at("type").get_to(p.type);
    }

    inline void from_json(const nlohmann::ordered_json& j, SDeviceRegistrationResponse& p)
    {
        j.at("data").at("deviceAccessKey").get_to(p.deviceAccessKey);
        j.at("data").at("deviceSecretKey").get_to(p.deviceSecretKey);
        j.at("data").at("settings").at("backupDate").get_to(p.settings.backupDate);
        j.at("data").at("settings").at("identifier").get_to(p.settings.identifier);
        j.at("data").at("settings").at("time").get_to(p.settings.time);
        j.at("data").at("settings").at("content").get_to(p.settings.content);
        j.at("data").at("settings").at("type").get_to(p.settings.type);
        j.at("data").at("settings").at("action").get_to(p.settings.action);

        if (j.at("data").contains("sharingKeys"))
        {
            j.at("data").at("sharingKeys").at("privateKey").get_to(p.sharingKeys.privateKey);
            j.at("data").at("sharingKeys").at("publicKey").get_to(p.sharingKeys.publicKey);
        }

        if (j.at("data").contains("remoteKeys"))
        {
            j.at("data").at("remoteKeys").get_to(p.remoteKeys);
        }

        j.at("data").at("numberOfDevices").get_to(p.numberOfDevices);
        j.at("data").at("hasDesktopDevices").get_to(p.hasDesktopDevices);
        j.at("data").at("publicUserId").get_to(p.publicUserId);
        j.at("data").at("userAnalyticsId").get_to(p.userAnalyticsId);
        j.at("data").at("deviceAnalyticsId").get_to(p.deviceAnalyticsId);

        if (j.at("data").contains("ssoServerKey"))
        {
            j.at("data").at("ssoServerKey").get_to(p.ssoServerKey);
        }

        if (j.at("data").contains("serverKey"))
        {
            j.at("data").at("serverKey").get_to(p.serverKey);
        }
    }

	inline EDashlaneError CompleteDeviceRegistration(DashlaneContextInternal& context, const std::string& authTicket, Dashlane::SDeviceRegistrationResponse& response)
	{
		nlohmann::ordered_json result;
		EDashlaneError rc = Dashlane::RequestApi(
			context,
			"authentication/CompleteDeviceRegistrationWithAuthTicket",
			result,
			{
				{"device", {
					{"deviceName", "Dashlane CLI"},
					{"appVersion", "1.5.0-cli"},
					{"platform", "server_standalone"},
					{"osCountry", "en_US"},
					{"osLanguage", "en_US"},
					{"temporary", false}
				}},
				{"login", context.login},
				{"authTicket", authTicket}
			}
		);

		if (rc == EDashlaneError::NoError)
		{
			response = result.get<Dashlane::SDeviceRegistrationResponse>();
		}

		return rc;
	}

}