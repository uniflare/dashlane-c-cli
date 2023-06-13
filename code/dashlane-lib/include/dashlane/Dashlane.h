#pragma once

#include "API.h"
#include "Errors.h"

struct DashlaneContext {};
struct DashlaneQueryContext {};

enum class ETransactionType
{
	Unknown,
	Authentifiant,
	SecureNote
};

extern "C"
{
	typedef void(*Dash_QueryWriterFunc)(void* pUserPointer, const char* json, uint32_t size);

	// Used to get the human readable error message of an error code returned by one of the library functions
	DASHLANE_API const char* Dash_GetErrorMessage(uint32_t errorCode);

	// Initialize the Dashlane context used for all operations
	DASHLANE_API uint32_t Dash_InitContext(DashlaneContext** ppContext,
		const char* szApplicationName,
		const char* szLogin,
		const char* szAppAccessKey,
		const char* szAppSecretKey);

	// Initialize the Query context used for querying transactions
	DASHLANE_API uint32_t Dash_InitQueryContext(DashlaneQueryContext** ppQueryContext);

	// Should be called once you are finished with any specific Dashlane context
	DASHLANE_API void Dash_FreeContext(DashlaneContext* pContext);

	// Should be called once you are finished with any specific Query context
	DASHLANE_API void Dash_FreeQueryContext(DashlaneQueryContext* pQueryContext);

	// Assigns the Master Password to the provided context
	DASHLANE_API uint32_t Dash_AssignMasterPassword(DashlaneContext* pContext, const char* szMasterPassword);

	// Assigns the Email Token to the provided context
	DASHLANE_API uint32_t Dash_AssignEmailToken(DashlaneContext* pContext, const char* szEmailToken);

	// Assigns the 2FA code to the provided context
	DASHLANE_API uint32_t Dash_Assign2FACode(DashlaneContext* pContext, const char* sz2FACode);

	// Clears the Master Password from the provided context
	DASHLANE_API void Dash_ClearMasterPassword(DashlaneContext* pContext);

	// Clears the Email Token from the provided context
	DASHLANE_API void Dash_ClearEmailToken(DashlaneContext* pContext);

	// Clears the 2FA code from the provided context
	DASHLANE_API void Dash_Clear2FACode(DashlaneContext* pContext);

	// Used with QueryTransactions, types is a 32bit mask of transaction types
	DASHLANE_API uint32_t Dash_AddQueryTransactionTypes(DashlaneQueryContext* pQueryContext, uint32_t types);

	// Used with QueryTransactions, szName is a field name, and szWildcard is a wildcard string to match against
	// This is a white-list approach, so only transactions that have the fields and match will be found
	DASHLANE_API uint32_t Dash_AddQueryFilter(DashlaneQueryContext* pQueryContext, const char* szName, const char* szWildcard);

	// Set the query writer function, must be set before QueryTransactions is called
	DASHLANE_API uint32_t Dash_SetQueryWriter(DashlaneQueryContext* pQueryContext, Dash_QueryWriterFunc writer, void* pUserPointer = nullptr);

	// After applying filters, try to find matching transactions (Passwords/Secure Notes etc...)
	DASHLANE_API uint32_t Dash_QueryTransactions(DashlaneContext* pContext, DashlaneQueryContext* pQueryContext);

	// Synchronizing the vault data must be done before querying transactions
	DASHLANE_API uint32_t Dash_SynchronizeVaultData(DashlaneContext* pContext);

	// Resets/Removes the vault data and any stored keys, and resets the configuration
	DASHLANE_API uint32_t Dash_ResetVaultData(DashlaneContext* pContext, bool removeAllUsers = false);

	// Set config option for storing the Master Password in the OS keychain
	DASHLANE_API uint32_t Dash_SetShouldStoreMasterPassword(DashlaneContext* pContext, bool shouldStoreMasterPassword);

	// Get the current config option value for storing the Master Password in the OS keychain
	DASHLANE_API uint32_t Dash_GetShouldStoreMasterPassword(DashlaneContext* pContext, bool* pShouldStoreMasterPasswordOut);

	// Set config option for automatically synchronizing if the last sync was over an hour ago
	DASHLANE_API uint32_t Dash_SetAutoSync(DashlaneContext* pContext, bool autoSync);

	// Get the current config option value for automatically synchronizing if the last sync was over an hour ago
	DASHLANE_API uint32_t Dash_GetAutoSync(DashlaneContext* pContext, bool* pAutoSync);
}