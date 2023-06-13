#pragma once

#include <dashlane/Dashlane.h>
#include "Database.h"

namespace Dashlane
{


	struct DashlaneQueryContextInternal : public DashlaneQueryContext
	{
		DashlaneQueryContextInternal()
			: typeMask(bitmask<Dashlane::ERawTransactionType>::none())
			, filters()
			, writerFunc(nullptr)
		{}

		bitmask<Dashlane::ERawTransactionType> typeMask{bitmask<Dashlane::ERawTransactionType>::none()};
		std::map<std::string, std::string> filters{};
		Dash_QueryWriterFunc writerFunc{ nullptr };
		void* pUserPointer{ nullptr };
	};

	struct DashlaneContextInternal : public DashlaneContext
	{
		DashlaneContextInternal(const char* szLogin, const char* szApplicationName)
			: applicationName(szApplicationName)
			, login(szLogin)
			, pDatabase(nullptr)
		{}

		const std::string applicationName;
		const std::string login;
		std::unique_ptr<Dashlane::CDatabase> pDatabase;

		struct
		{
			std::string masterPassword;
			struct {
				std::string accessKey;
				std::string secretKey;
			} app;
			struct {
				std::string accessKey;
				std::string secretKey;
			} device;
			std::vector<uint8_t> localKey;
			std::string serverKey;
			std::string twoFactorCode;
			std::string emailToken;
		} secrets;

		struct {
			bool shouldUpdateDeviceConfiguration{ false };
		} applicationData;
	};

	EDashlaneError EncryptAndSerialize(
		const DashlaneContextInternal& context,
		const std::vector<uint8_t>& input,
		std::string& output
	);
	EDashlaneError DeserializeAndDecrypt(
		const DashlaneContextInternal& context,
		const std::string& input,
		std::vector<uint8_t>& output
	);

}