#include "StdAfx.h"
#include "Keychain.h"

#include "Dashlane.h"
#include "Encryption.h"
#include "Utility/Cryptography.h"
#include "Utility/Strings.h"
#include "Utility/Vector.h"

#include "Api/Endpoints/CompleteDeviceRegistration.h"
#include "Api/Endpoints/GetAuthenticationMethodsForDevice.h"
#include "Api/Endpoints/PerformDuoPushVerification.h"
#include "Api/Endpoints/PerformDashlaneAuthenticatorVerification.h"
#include "Api/Endpoints/PerformTotpVerification.h"
#include "Api/Endpoints/RequestEmailTokenVerification.h"
#include "Api/Endpoints/PerformEmailTokenVerification.h"

#include <keychain/keychain.h>

namespace Dashlane
{

	bool SetLocalKey(DashlaneContextInternal& context)
	{
		keychain::Error error;

		const std::string encodedKey = base64pp::encode(context.secrets.localKey);
		keychain::setPassword(context.applicationName, context.login, context.login, encodedKey, error);

		return error.type == keychain::ErrorType::NoError;
	}

	bool GetLocalKey(DashlaneContextInternal& context)
	{
		keychain::Error error;

		const std::string key = keychain::getPassword(context.applicationName, context.login, context.login, error);
		if (error.type == keychain::ErrorType::NoError)
		{
			context.secrets.localKey = base64pp::decode(key).value();
			return true;
		}
		return false;
	}

	bool DeleteLocalKey(DashlaneContextInternal& context)
	{
		keychain::Error error;
		keychain::deletePassword(context.applicationName, context.login, context.login, error);
		return error.type == keychain::ErrorType::NoError;
	}

	Dashlane::SEncryptedData GetDerivationParametersForLocalKey(const DashlaneContextInternal& context)
	{
		Dashlane::SEncryptedData ed;

		ed.pKeyDerivation = std::make_unique<Dashlane::SDerivationConfigArgon2>();
		auto& config = static_cast<Dashlane::SDerivationConfigArgon2&>(*ed.pKeyDerivation);
		config.saltLength = 16;
		config.tCost = 3;
		config.mCost = 32768;
		config.parallelism = 2;

		ed.cipherConfig.cipherMode = Dashlane::ECipherMode::CBCHMAC;
		ed.cipherConfig.ivLength = 0;

		ed.cipherData.salt = Utility::SliceVectorByCopy(Utility::SHA512(std::vector<uint8_t>(context.login.begin(), context.login.end())), 0, 16);

		return ed;
	}

	EDashlaneError GetSymmetricKey(DashlaneContextInternal& context, std::vector<uint8_t>& symmetricKey)
	{
		if (context.secrets.masterPassword.empty())
		{
			return EDashlaneError::RequireMasterPassword;
		}

		Dashlane::CEncryption encryption;
		const Dashlane::SEncryptedData derivateParameters = GetDerivationParametersForLocalKey(context);
		if (!encryption.GetSymmetricKeyViaDerivate(
			*derivateParameters.pKeyDerivation,
			derivateParameters.cipherData.salt,
			context.secrets.masterPassword,
			symmetricKey
		))
		{
			return EDashlaneError::InternalEncryptFailure;
		}

		return EDashlaneError::NoError;
	}

	EDashlaneError RegisterDeviceWithAPI(DashlaneContextInternal& context)
	{
		EDashlaneError rc = EDashlaneError::NoError;

		std::vector<std::unique_ptr<Dashlane::IAuthenticationMethodBase>> methods;
		rc = GetAuthenticationMethodsForDevice(context, methods);

		if (rc != EDashlaneError::NoError)
			return rc;

		if (methods.size() == 0)
			return EDashlaneError::NoSupportedAuth;

		std::string authTicket;
		switch (methods.at(0)->type)
		{

		case Dashlane::EAuthMethod::DuoPush:
			rc = PerformDuoPushVerification(context, authTicket);

			if (rc == EDashlaneError::AuthenticationFailed)
				return EDashlaneError::DuoPushVerificationFailed;
			break;

		case Dashlane::EAuthMethod::DashlaneAuthenticator:
			rc = PerformDashlaneAuthenticatorVerification(context, authTicket);

			if (rc == EDashlaneError::AuthenticationFailed)
				return EDashlaneError::AuthenticatorVerificationFailed;

			break;

		case Dashlane::EAuthMethod::Totp:
			if (context.secrets.twoFactorCode.empty())
				return EDashlaneError::Require2FACode;

			rc = PerformTotpVerification(context, authTicket);

			if (rc == EDashlaneError::AuthenticationFailed)
			{
				return EDashlaneError::Invalid2FACode;
			}

			context.secrets.twoFactorCode.clear();

			break;

		case Dashlane::EAuthMethod::EmailToken:
			if (context.secrets.emailToken.empty())
			{
				RequestEmailTokenVerification(context);
				return EDashlaneError::RequireEmailToken;
			}

			rc = PerformEmailTokenVerification(context, authTicket);

			if (rc == EDashlaneError::AuthenticationFailed)
			{
				return EDashlaneError::InvalidEmailToken;
			}

			context.secrets.emailToken.clear();

			break;

		}

		if (!authTicket.empty())
		{
			Dashlane::SDeviceRegistrationResponse resp;
			rc = CompleteDeviceRegistration(context, authTicket, resp);
			if (rc == EDashlaneError::NoError)
			{
				context.secrets.device.accessKey = resp.deviceAccessKey;
				context.secrets.device.secretKey = resp.deviceSecretKey;
				context.secrets.serverKey = resp.serverKey;

				return EDashlaneError::NoError;
			}
		}

		return rc;
	}

	EDashlaneError GetDeviceConfiguration(DashlaneContextInternal& context)
	{
		Dashlane::SDeviceConfiguration deviceConfig;
		if (!context.pDatabase->GetDeviceConfiguration(context, deviceConfig))
			return EDashlaneError::DeviceNotRegistered;

		if (deviceConfig.accessKey.empty())
			return EDashlaneError::DeviceNotRegistered;

		if (deviceConfig.secretKeyEncrypted.empty())
			return EDashlaneError::DeviceNotRegistered;

		context.secrets.device.accessKey = deviceConfig.accessKey;

		std::vector<uint8_t> secretKeyDecrypted;
		if (EDashlaneError rc = DeserializeAndDecrypt(context, deviceConfig.secretKeyEncrypted, secretKeyDecrypted); rc != EDashlaneError::NoError)
			// Local key does not decrypt data, device requires re-registration
			return EDashlaneError::DeviceNotRegistered;
		context.secrets.device.secretKey = Utility::ToHex(secretKeyDecrypted);

		if (!deviceConfig.masterPasswordEncrypted.empty())
		{
			std::vector<uint8_t> masterPasswordDecrypted;
			if (EDashlaneError rc = DeserializeAndDecrypt(context, deviceConfig.masterPasswordEncrypted, masterPasswordDecrypted); rc != EDashlaneError::NoError)
				return rc;
			context.secrets.masterPassword = Utility::VectorU8ToString(masterPasswordDecrypted);
		}

		if (!deviceConfig.serverKeyEncrypted.empty())
		{
			std::vector<uint8_t> serverKeyDecrypted;
			if (EDashlaneError rc = DeserializeAndDecrypt(context, deviceConfig.serverKeyEncrypted, serverKeyDecrypted); rc != EDashlaneError::NoError)
				return rc;
			context.secrets.serverKey = Utility::VectorU8ToString(serverKeyDecrypted);
		}

		return EDashlaneError::NoError;
	}

	EDashlaneError GetLocalKeyFromDatabase(DashlaneContextInternal& context)
	{
		EDashlaneError rc = EDashlaneError::NoError;

		Dashlane::SDeviceConfiguration deviceConfig;
		if (context.pDatabase->GetDeviceConfiguration(context, deviceConfig))
		{
			if (context.secrets.masterPassword.empty())
				return EDashlaneError::RequireMasterPassword;

			std::vector<uint8_t> symmetricKey;
			rc = GetSymmetricKey(context, symmetricKey);

			if (rc == EDashlaneError::NoError)
			{
				// Use symmetricKey as localKey for crypto context
				context.secrets.localKey = symmetricKey;
				const std::vector<uint8_t> localKeyEncrypted = Utility::StringToVectorU8(deviceConfig.localKeyEncrypted);
				rc = DeserializeAndDecrypt(context, deviceConfig.localKeyEncrypted, context.secrets.localKey);
			}
		}

		return rc;
	}

	EDashlaneError UpdateDeviceConfiguration(DashlaneContextInternal& context)
	{
		Dashlane::SDeviceConfiguration deviceConfig;
		deviceConfig.login = context.login;
		deviceConfig.version = context.applicationName;
		deviceConfig.accessKey = context.secrets.device.accessKey;

		// Store everything else encrypted
		if (!context.secrets.serverKey.empty())
		{
			EDashlaneError rc = EncryptAndSerialize(context, Utility::StringToVectorU8(context.secrets.serverKey), deviceConfig.serverKeyEncrypted);
			if (rc != EDashlaneError::NoError)
				return EDashlaneError::InternalEncryptFailure;
		}

		{
			EDashlaneError rc = EncryptAndSerialize(context, Utility::FromHex(context.secrets.device.secretKey), deviceConfig.secretKeyEncrypted);
			if (rc != EDashlaneError::NoError)
				return EDashlaneError::InternalEncryptFailure;
		}

		// Silent failure will resort to default value
		bool shouldSaveMasterPassword = false;
		Dash_GetShouldStoreMasterPassword(&context, &shouldSaveMasterPassword);
		deviceConfig.shouldNotSaveMasterPassword = !shouldSaveMasterPassword;

		if (!deviceConfig.shouldNotSaveMasterPassword)
		{
			EDashlaneError rc = EncryptAndSerialize(context, Utility::StringToVectorU8(context.secrets.masterPassword), deviceConfig.masterPasswordEncrypted);
			if (rc != EDashlaneError::NoError)
				return EDashlaneError::InternalEncryptFailure;
		}

		// Use symmetricKey to encrypt the localKey
		{
			const std::vector<uint8_t> localKey = context.secrets.localKey;
			EDashlaneError rc = GetSymmetricKey(context, context.secrets.localKey);
			if (rc != EDashlaneError::NoError)
				return rc;

			rc = EncryptAndSerialize(context, context.secrets.localKey, deviceConfig.localKeyEncrypted);
			if (rc != EDashlaneError::NoError)
				return EDashlaneError::InternalEncryptFailure;
			context.secrets.localKey = localKey;
		}

		// TODO: Leftover device properties
		//bool        autoSync;

		if (!context.pDatabase->SetDeviceConfiguration(deviceConfig))
			return EDashlaneError::DatabaseTransactionFailure;

		return EDashlaneError::NoError;
	}

	// Attempts to fill context with required secrets for the Vault/API
	EDashlaneError GetOrUpdateSecrets(DashlaneContextInternal& context)
	{
		if (!GetLocalKey(context))
		{
			EDashlaneError rc = GetLocalKeyFromDatabase(context);
			if (rc != EDashlaneError::NoError)
				return rc;

			if (context.secrets.localKey.empty())
			{
				// Generate new local key (database will need to be re-synchronized with new key)
				context.secrets.localKey = Utility::GenerateRandomData(32);
			}

			SetLocalKey(context);
		}

		if (context.secrets.masterPassword.empty())
		{
			return EDashlaneError::RequireMasterPassword;
		}

		if (context.secrets.device.accessKey.empty())
		{
			EDashlaneError rc = GetDeviceConfiguration(context);
			if (rc == EDashlaneError::DeviceNotRegistered)
			{
				rc = RegisterDeviceWithAPI(context);
				if (rc != EDashlaneError::NoError)
					return rc;

				// NOTE: We should defer recording the device config in case the password is wrong
				// TODO: We can lways ask for password again, and if device pass != context pass, we prefer context
				context.applicationData.shouldUpdateDeviceConfiguration = true;
			}
		}

		// Used with TOTP type authentication data
		if (!context.secrets.serverKey.empty() && !context.secrets.masterPassword.starts_with(context.secrets.serverKey))
		{
			context.secrets.masterPassword = context.secrets.serverKey + context.secrets.masterPassword;
		}

		SDeviceConfiguration config;
		context.pDatabase->GetDeviceConfiguration(context, config);

		if (!config.shouldNotSaveMasterPassword && config.masterPasswordEncrypted.empty())
		{
			context.applicationData.shouldUpdateDeviceConfiguration = true;
		}

		return EDashlaneError::NoError;
	}

}