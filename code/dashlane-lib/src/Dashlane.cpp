#include "StdAfx.h"
#include "Dashlane.h"
#include "Encryption.h"
#include "Keychain.h"
#include "Api/Endpoints/GetLatestContent.h"
#include "Types/Transactions.h"
#include "Utility/Strings.h"
#include "Utility/Time.h"
#include "Utility/Transaction.h"
#include "Utility/Vector.h"
#include "Utility/Zip.h"

#define ENSURE_POINTER(ptr, rc_error) 			   \
	if (ptr == nullptr) return RC_TO_INT(rc_error) \

#define ENSURE_STRLEN(str, rc_error) 				      \
	if (std::strlen(str) == 0) return RC_TO_INT(rc_error) \

#define ENSURE_POINTER_VOID(ptr) \
	if (ptr == nullptr) return   \

#define ENSURE_STRLEN_VOID(str) 	  \
	if (std::strlen(str) == 0) return \

inline uint32_t RC_TO_INT(EDashlaneError rc)
{
	return static_cast<uint32_t>(rc);
}

namespace Dashlane
{

	EDashlaneError EncryptAndSerialize(
		const DashlaneContextInternal& context,
		const std::vector<uint8_t>& input,
		std::string& output
	)
	{
		// Encrypt
		Dashlane::CEncryption encryptor;
		Dashlane::SEncryptedData encryptedDataOut;
		if (!encryptor.EncryptData(context.secrets.localKey, input, encryptedDataOut))
		{
			return EDashlaneError::InternalEncryptFailure;
		}

		// Serialize
		std::vector<uint8_t> serialized;
		CSerializer::DoSerialize(serialized, encryptedDataOut);

		// Encode
		output = base64pp::encode(serialized);

		return EDashlaneError::NoError;
	}

	EDashlaneError DeserializeAndDecrypt(
		const DashlaneContextInternal& context,
		const std::string& input,
		std::vector<uint8_t>& output
	)
	{
		// Decode
		const auto maybeDecoded = base64pp::decode(input);
		if (maybeDecoded == std::nullopt)
		{
			return EDashlaneError::InternalDecryptFailure;
		}

		// Deserialize
		Dashlane::SEncryptedData encryptedData;
		CSerializer::DoDeserialize(maybeDecoded.value(), encryptedData);

		// Decrypt
		Dashlane::CEncryption encryption;

		std::vector<uint8_t> symmetricKey;
		if (encryptedData.pKeyDerivation->GetDerivation() != Dashlane::EDerivationAlgorithm::None)
		{
			EDashlaneError rc = encryption.GetSymmetricKeyFromData(context, maybeDecoded.value(), encryptedData, symmetricKey);
			if (rc != EDashlaneError::NoError)
				return rc;
		}
		else
		{
			symmetricKey = context.secrets.localKey;
		}

		if (!encryption.SetContextFromEncryptedData(symmetricKey, maybeDecoded.value(), encryptedData))
		{
			return EDashlaneError::InternalDecryptFailure;
		}

		if (!encryption.DecryptFromContext())
		{
			return EDashlaneError::InternalDecryptFailure;
		}

		output = encryption.GetOutput();

		return EDashlaneError::NoError;
	}

	EDashlaneError RecryptTransactionContent(const DashlaneContextInternal& context, const std::string& content, std::string& output)
	{
		// Decode, Deserialize, Decrypt
		std::vector<uint8_t> decrypted;

		if (EDashlaneError rc = DeserializeAndDecrypt(context, content, decrypted); rc != EDashlaneError::NoError)
		{
			return rc;
		}

		if (EDashlaneError rc = EncryptAndSerialize(context, decrypted, output); rc != EDashlaneError::NoError)
		{
			return rc;
		}

		return EDashlaneError::NoError;
	}

	EDashlaneError ProcessTransaction(const DashlaneContextInternal& context, const Dashlane::SRawTransactionBackupEdit& transaction, nlohmann::ordered_json& jsonOut)
	{
		// Decode, Deserialize, Decrypt
		std::vector<uint8_t> decrypted;
		if (EDashlaneError rc = DeserializeAndDecrypt(context, transaction.content, decrypted); rc != EDashlaneError::NoError)
		{
			return rc;
		}

		// Decompress
		const auto compressed = Utility::SliceVectorBySpan(decrypted, 6);
		const std::vector<uint8_t> decompressed = Utility::InflateRaw(compressed);

		// XML to Json
		jsonOut = Utility::XmlToJsonTransaction(decompressed);

		return EDashlaneError::NoError;
	}

	static std::vector<std::shared_ptr<DashlaneContextInternal>> s_contexts = {};
	static std::vector<std::shared_ptr<DashlaneQueryContextInternal>> s_queryContexts = {};

}

const char* Dash_GetErrorMessage(uint32_t errorCode)
{
	switch ((EDashlaneError)errorCode)
	{
	case EDashlaneError::NoError: return "Function executed successfully";
		break;

	case EDashlaneError::RequireMasterPassword: return "The context requires the Master Password to be set to continue";
		break;
	case EDashlaneError::Require2FACode: return "The context requires the 2FA code to be set to continue";
		break;
	case EDashlaneError::RequireEmailToken: return "The context requires the email token to be set to continue";
		break;

	case EDashlaneError::InvalidContext: return "The provided context is invalid";
		break;
	case EDashlaneError::MissingOrInvalidLogin: return "Cannot create Dashlane context with an empty Login parameter";
		break;
	case EDashlaneError::MissingOrInvalidAppKeys: return "Cannot create Dashlane context without valid App Keys";
		break;

	case EDashlaneError::Invalid2FACode: return "The Two-Factor authentication code provided is invalid";
		break;
	case EDashlaneError::InvalidEmailToken: return "The email token provided is invalid";
		break;
	case EDashlaneError::InvalidMasterPassword: return "The Master Password provided is invalid";
		break;
	case EDashlaneError::DeviceRegistrationFailed: return "Failed to register device (Incorrect authentication?)";
		break;

	case EDashlaneError::InvalidParameter: return "An invalid parameter was provided to a library function";
		break;
	case EDashlaneError::DeviceNotRegistered: return "This device has not been registered, this function is not available";
		break;

	case EDashlaneError::InvalidAPIRequest: return "An internal error occurred and the API request failed";
		break;
	case EDashlaneError::NoSupportedAuth: return "No authentication methods available that are supported by this application";
		break;
	case EDashlaneError::FailedDatabaseConnection: return "Failed to create SQLite connection (Invalid path?)";
		break;
	case EDashlaneError::FailedDatabaseCreation: return "Failed to create SQLite tables or communicate with SQLite";
		break;
	case EDashlaneError::KeychainUnavailable: return "OS Keychain not available on this system, Master Password cannot be stored";
		break;
	case EDashlaneError::UpdateConfigFailed: return "Failed to update Dashlane configuration in database";
		break;
	case EDashlaneError::APIError_Timeout: return "The API request was not in time for the API and we couldn't recover";
		break;
	}

	return "Unknown error code";
}

uint32_t Dash_InitContext(DashlaneContext** ppContext,
	const char* szApplicationName,
	const char* szLogin,
	const char* szAppAccessKey,
	const char* szAppSecretKey)
{
	if (ppContext == nullptr ||
		std::strlen(szApplicationName) == 0 ||
		std::strlen(szLogin) == 0 ||
		std::strlen(szAppAccessKey) == 0 ||
		std::strlen(szAppSecretKey) == 0)
	{
		return RC_TO_INT(EDashlaneError::InvalidParameter);
	}

	auto pContext = Dashlane::s_contexts.emplace_back(std::make_shared<Dashlane::DashlaneContextInternal>(szLogin, szApplicationName));

	pContext->secrets.app.accessKey = szAppAccessKey;
	pContext->secrets.app.secretKey = szAppSecretKey;

	*ppContext = pContext.get();

	pContext->pDatabase = std::make_unique<Dashlane::CDatabase>();
	if (!pContext->pDatabase->Connect())
	{
		return RC_TO_INT(EDashlaneError::FailedDatabaseConnection);
	}

	if (!pContext->pDatabase->Prepare())
	{
		return RC_TO_INT(EDashlaneError::FailedDatabaseCreation);
	}

	return RC_TO_INT(EDashlaneError::NoError);
}

uint32_t Dash_InitQueryContext(DashlaneQueryContext** ppQueryContext)
{
	if (ppQueryContext == nullptr)
		return RC_TO_INT(EDashlaneError::InvalidParameter);

	*ppQueryContext = Dashlane::s_queryContexts.emplace_back(std::make_shared<Dashlane::DashlaneQueryContextInternal>()).get();
	return RC_TO_INT(EDashlaneError::NoError);
}

void Dash_FreeContext(DashlaneContext* pContext)
{
	if (pContext != nullptr)
	{
		std::erase_if(Dashlane::s_contexts, [pContext](std::shared_ptr<Dashlane::DashlaneContextInternal> pElem)
		{
			return pElem.get() == static_cast<Dashlane::DashlaneContextInternal*>(pContext);
		});
	}
}

void Dash_FreeQueryContext(DashlaneQueryContext* pQueryContext)
{
	if (pQueryContext != nullptr)
	{
		std::erase_if(Dashlane::s_queryContexts, [pQueryContext](std::shared_ptr<Dashlane::DashlaneQueryContextInternal> pElem)
		{
			return pElem.get() == static_cast<Dashlane::DashlaneQueryContextInternal*>(pQueryContext);
		});
	}
}

uint32_t Dash_AssignMasterPassword(DashlaneContext* pContext, const char* szMasterPassword)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);

	ENSURE_POINTER(pInternalContext, EDashlaneError::InvalidContext);
	ENSURE_STRLEN(szMasterPassword, EDashlaneError::InvalidParameter);

	pInternalContext->secrets.masterPassword = szMasterPassword;
	return RC_TO_INT(EDashlaneError::NoError);
}

uint32_t Dash_AssignEmailToken(DashlaneContext* pContext, const char* szEmailToken)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);

	ENSURE_POINTER(pInternalContext, EDashlaneError::InvalidContext);
	ENSURE_STRLEN(szEmailToken, EDashlaneError::InvalidParameter);

	pInternalContext->secrets.emailToken = szEmailToken;
	return RC_TO_INT(EDashlaneError::NoError);
}

uint32_t Dash_Assign2FACode(DashlaneContext* pContext, const char* sz2FACode)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);

	ENSURE_POINTER(pInternalContext, EDashlaneError::InvalidContext);
	ENSURE_STRLEN(sz2FACode, EDashlaneError::InvalidParameter);

	pInternalContext->secrets.twoFactorCode = sz2FACode;
	return RC_TO_INT(EDashlaneError::NoError);
}

void Dash_ClearMasterPassword(DashlaneContext* pContext)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);

	ENSURE_POINTER_VOID(pInternalContext);

	pInternalContext->secrets.masterPassword.clear();
}

void Dash_ClearEmailToken(DashlaneContext* pContext)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);

	ENSURE_POINTER_VOID(pInternalContext);

	pInternalContext->secrets.emailToken.clear();
}

void Dash_Clear2FACode(DashlaneContext* pContext)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);

	ENSURE_POINTER_VOID(pInternalContext);

	pInternalContext->secrets.twoFactorCode.clear();
}

uint32_t Dash_AddQueryTransactionTypes(DashlaneQueryContext* pQueryContext, uint32_t types)
{
	auto pInternalQueryContext = static_cast<Dashlane::DashlaneQueryContextInternal*>(pQueryContext);

	ENSURE_POINTER(pInternalQueryContext, EDashlaneError::InvalidContext);

#ifdef _DEBUG
	if (types > bitmask<Dashlane::ERawTransactionType>::all().value)
		return RC_TO_INT(EDashlaneError::InvalidParameter);
#endif

	pInternalQueryContext->typeMask |= (Dashlane::ERawTransactionType)types;

	return RC_TO_INT(EDashlaneError::NoError);
}

uint32_t Dash_AddQueryFilter(DashlaneQueryContext* pQueryContext, const char* szName, const char* szWildcard)
{
	auto pInternalQueryContext = static_cast<Dashlane::DashlaneQueryContextInternal*>(pQueryContext);

	ENSURE_POINTER(pInternalQueryContext, EDashlaneError::InvalidContext);
	ENSURE_STRLEN(szName, EDashlaneError::InvalidParameter);

	pInternalQueryContext->filters[szName] = szWildcard;

	return RC_TO_INT(EDashlaneError::NoError);
}

uint32_t Dash_SetQueryWriter(DashlaneQueryContext* pQueryContext, Dash_QueryWriterFunc writer, void* pUserPointer)
{
	auto pInternalQueryContext = static_cast<Dashlane::DashlaneQueryContextInternal*>(pQueryContext);

	ENSURE_POINTER(pInternalQueryContext, EDashlaneError::InvalidContext);
	ENSURE_POINTER(writer, EDashlaneError::InvalidParameter);

	pInternalQueryContext->writerFunc = writer;
	pInternalQueryContext->pUserPointer = pUserPointer;

	return RC_TO_INT(EDashlaneError::NoError);
}

uint32_t Dash_SynchronizeVaultData(DashlaneContext* pContext)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);

	ENSURE_POINTER(pInternalContext, EDashlaneError::InvalidContext);

	EDashlaneError rc = GetOrUpdateSecrets(*pInternalContext);

	if (rc == EDashlaneError::NoError)
	{
		Dashlane::SGetLatestContentResponse latestContent;
		rc = Dashlane::GetLatestContent(*pInternalContext, pInternalContext->pDatabase->GetLastSyncTime(*pInternalContext), latestContent);
		if (rc == EDashlaneError::NoError)
		{
			std::vector<Dashlane::STransactionRow> rows;

			for (const auto& transactionBase : latestContent.transactions)
			{
				switch (transactionBase->GetAction())
				{

				case Dashlane::ETransactionAction::BackupEdit:
				{
					const std::vector<uint8_t> encryptedContent = Utility::StringToVectorU8(transactionBase->GetContent());
					std::string recryptedContent;
					if (rc = RecryptTransactionContent(*pInternalContext, transactionBase->GetContent(), recryptedContent); rc != EDashlaneError::NoError)
					{
						// Most likely, master password is incorrect
						pInternalContext->secrets.masterPassword.clear();
						return RC_TO_INT(EDashlaneError::InvalidMasterPassword);
					}

					rows.emplace_back(
						pInternalContext->login,
						transactionBase->GetIdentifier(),
						transactionBase->GetType(),
						transactionBase->GetActionName(),
						recryptedContent);
				} break;

				case Dashlane::ETransactionAction::BackupRemove:
				{
					const auto& transaction = static_cast<const Dashlane::SRawTransactionBackupRemove&>(*transactionBase);
					rows.emplace_back(
						pInternalContext->login,
						transactionBase->GetIdentifier(),
						transactionBase->GetType(),
						transactionBase->GetActionName(),
						""
					);
				} break;

				}
			}

			if (!pInternalContext->pDatabase->AddMultipleTransactionData(rows))
				return RC_TO_INT(EDashlaneError::DatabaseTransactionFailure);

			if (!pInternalContext->pDatabase->UpdateLastSyncTime(*pInternalContext, latestContent.timestamp))
				return RC_TO_INT(EDashlaneError::DatabaseTransactionFailure);
		}

		if (pInternalContext->applicationData.shouldUpdateDeviceConfiguration)
		{
			rc = UpdateDeviceConfiguration(*pInternalContext);
		}
	}
	else
	{
		if (rc == EDashlaneError::InvalidMasterPassword)
			pInternalContext->secrets.masterPassword.clear();
	}

	return RC_TO_INT(rc);
}

std::string::const_iterator FindCaseInsensitive(const std::string& haystack, const std::string& needle)
{
	return std::search(haystack.cbegin(), haystack.cend(), needle.cbegin(), needle.cend(),
		[](const char lhs, const char rhs) { return std::tolower(lhs) == std::tolower(rhs); });
}

bool FindAnyValue(const nlohmann::ordered_json& json, const std::string& value)
{
	for (const auto& element : json.items())
	{
		const std::string elementValue = element.value().get<std::string>();
		if (FindCaseInsensitive(elementValue, value) != elementValue.cend())
			return true;
	}

	return false;
}

bool FindKeyValue(const nlohmann::ordered_json& json, const std::string& key, const std::string& value)
{
	if (json.contains(key))
	{
		std::string elementValue = json.find(key).value().get<std::string>();
		if (FindCaseInsensitive(elementValue, value) != elementValue.cend())
			return true;
	}

	return false;
}

uint32_t Dash_QueryTransactions(DashlaneContext* pContext, DashlaneQueryContext* pQueryContext)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);
	auto pInternalQueryContext = static_cast<Dashlane::DashlaneQueryContextInternal*>(pQueryContext);

	ENSURE_POINTER(pInternalContext, EDashlaneError::InvalidContext);
	ENSURE_POINTER(pInternalQueryContext, EDashlaneError::InvalidContext);

	EDashlaneError rc = GetOrUpdateSecrets(*pInternalContext);
	if (rc != EDashlaneError::NoError)
	{
		if (rc == EDashlaneError::InvalidMasterPassword)
			pInternalContext->secrets.masterPassword.clear();

		return RC_TO_INT(rc);
	}

	Dashlane::SDeviceConfiguration config;
	const bool haveConfig = pInternalContext->pDatabase->GetDeviceConfiguration(*pInternalContext, config);
	if (config.autoSync || !haveConfig)
	{
		const uint64_t lastSyncTime = pInternalContext->pDatabase->GetLastSyncTime(*pInternalContext);
		const uint64_t nextSyncTime = lastSyncTime + 3600;
		if (nextSyncTime < Utility::GetUnixTimestamp())
		{
			rc = (EDashlaneError)Dash_SynchronizeVaultData(pContext);
			if (rc != EDashlaneError::NoError)
				return RC_TO_INT(rc);
		}
	}

	std::vector<Dashlane::SRawTransactionBackupEdit> transactions;
	rc = pInternalContext->pDatabase->GetTransactions(*pInternalContext, pInternalQueryContext->typeMask, transactions);
	if (rc != EDashlaneError::NoError)
		return RC_TO_INT(rc);

	for (const Dashlane::SRawTransactionBackupEdit& transaction : transactions)
	{
		nlohmann::ordered_json json;
		rc = ProcessTransaction(*pInternalContext, transaction, json);
		if (rc != EDashlaneError::NoError)
		{
			if (rc == EDashlaneError::InvalidMasterPassword)
				pInternalContext->secrets.masterPassword.clear();

			return RC_TO_INT(rc);
		}

		bool isMatch = false;
		if (pInternalQueryContext->filters.size() > 0)
		{
			for (const auto& filter : pInternalQueryContext->filters)
			{
				const std::string& key = filter.first;
				const std::string& needle = filter.second;

				if (needle.size() == 0)
				{
					if (FindAnyValue(json, key))
						isMatch = true;
				}
				else
				{
					if (FindKeyValue(json, key, needle))
						isMatch = true;
				}

				if (isMatch)
					break;
			}
		}
		else
			isMatch = true;

		if (isMatch)
			pInternalQueryContext->writerFunc(pInternalQueryContext->pUserPointer, json.dump().c_str(), json.size());

		if (pInternalContext->applicationData.shouldUpdateDeviceConfiguration)
			rc = UpdateDeviceConfiguration(*pInternalContext);
	}

	return RC_TO_INT(rc);
}

uint32_t Dash_ResetVaultData(DashlaneContext* pContext, bool removeAllUsers)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);

	ENSURE_POINTER(pInternalContext, EDashlaneError::InvalidContext);
	ENSURE_POINTER(pInternalContext->pDatabase, EDashlaneError::InvalidContext);

	if (!removeAllUsers)
	{
		if (pInternalContext->login.empty())
		{
			return RC_TO_INT(EDashlaneError::InvalidContext);
		}
		else
		{
			DeleteLocalKey(*pInternalContext);
			pInternalContext->pDatabase->RemoveUserData(*pInternalContext);
		}
	}
	else
	{
		std::vector<std::string> users;
		pInternalContext->pDatabase->GetRegisteredUsers(users);

		for (const auto& user : users)
		{
			Dashlane::DashlaneContextInternal ctx(user.c_str(), pInternalContext->applicationName.c_str());
			DeleteLocalKey(ctx);
		}

		// Quicker to just drop all the tables and recreate them if needed
		pInternalContext->pDatabase->Drop();
	}

	return (uint32_t)EDashlaneError::NoError;
}

uint32_t Dash_SetShouldStoreMasterPassword(DashlaneContext* pContext, bool shouldStoreMasterPassword)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);

	ENSURE_POINTER(pInternalContext, EDashlaneError::InvalidContext);
	ENSURE_POINTER(pInternalContext->pDatabase, EDashlaneError::InvalidContext);

	Dashlane::SDeviceConfiguration config;
	if (!pInternalContext->pDatabase->GetDeviceConfiguration(*pInternalContext, config))
		return RC_TO_INT(EDashlaneError::DeviceNotRegistered);

	config.shouldNotSaveMasterPassword = !shouldStoreMasterPassword;

	if (!shouldStoreMasterPassword && !config.masterPasswordEncrypted.empty())
		config.masterPasswordEncrypted.clear();

	if (!pInternalContext->pDatabase->SetDeviceConfiguration(config))
		return RC_TO_INT(EDashlaneError::UpdateConfigFailed);

	return RC_TO_INT(EDashlaneError::NoError);
}

uint32_t Dash_GetShouldStoreMasterPassword(DashlaneContext* pContext, bool* pShouldStoreMasterPasswordOut)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);

	ENSURE_POINTER(pInternalContext, EDashlaneError::InvalidContext);
	ENSURE_POINTER(pInternalContext->pDatabase, EDashlaneError::InvalidContext);

	Dashlane::SDeviceConfiguration config;
	if (!pInternalContext->pDatabase->GetDeviceConfiguration(*pInternalContext, config))
		return RC_TO_INT(EDashlaneError::DeviceNotRegistered);

	*pShouldStoreMasterPasswordOut = !config.shouldNotSaveMasterPassword;

	return RC_TO_INT(EDashlaneError::NoError);
}

uint32_t Dash_SetAutoSync(DashlaneContext* pContext, bool autoSync)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);

	ENSURE_POINTER(pInternalContext, EDashlaneError::InvalidContext);
	ENSURE_POINTER(pInternalContext->pDatabase, EDashlaneError::InvalidContext);

	Dashlane::SDeviceConfiguration config;
	if (!pInternalContext->pDatabase->GetDeviceConfiguration(*pInternalContext, config))
		return RC_TO_INT(EDashlaneError::DeviceNotRegistered);

	config.autoSync = autoSync;

	if (!pInternalContext->pDatabase->SetDeviceConfiguration(config))
		return RC_TO_INT(EDashlaneError::UpdateConfigFailed);

	return RC_TO_INT(EDashlaneError::NoError);
}

uint32_t Dash_GetAutoSync(DashlaneContext* pContext, bool* pAutoSync)
{
	auto pInternalContext = static_cast<Dashlane::DashlaneContextInternal*>(pContext);

	ENSURE_POINTER(pInternalContext, EDashlaneError::InvalidContext);
	ENSURE_POINTER(pInternalContext->pDatabase, EDashlaneError::InvalidContext);

	Dashlane::SDeviceConfiguration config;
	if (!pInternalContext->pDatabase->GetDeviceConfiguration(*pInternalContext, config))
		return RC_TO_INT(EDashlaneError::DeviceNotRegistered);

	*pAutoSync = config.autoSync;

	return RC_TO_INT(EDashlaneError::NoError);
}