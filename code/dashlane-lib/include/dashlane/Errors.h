#pragma once

#include <stdint.h>

enum class EDashlaneError : uint32_t
{
	NoError,

	// Errors relating to further input requirements (context must be re-used)
	RequireMasterPassword = 100u,		// Set master password using SetMasterPassword(context, masterPassword)
	Require2FACode,						// Set the 2FA code using Set2FACode(context, twoFactorCode)
	RequireEmailToken,					// Set the Email token using SetEmailToken(context, token)

	// Errors relating to an invalid context (context must be re-created)
	InvalidContext = 200u,				// Context either does not exist or was destroyed
	MissingOrInvalidLogin,				// Invalid login (email)
	MissingOrInvalidAppKeys,			// Invalid application keys (appAccessKey, appSecretKey)

	// User input or data errors
	Invalid2FACode = 300u,				// Invalid TwoFactor auth code, you can set and try again
	InvalidEmailToken,					// Invalid Email Token, you can set and try again
	InvalidMasterPassword,				// Invalid Master Password, you can set and try again
	DuoPushVerificationFailed,			// DuoPush verification failed
	AuthenticatorVerificationFailed,	// Dashlane Authenticator verification failed
	DeviceRegistrationFailed,			// Failed to register device (Incorrect authentication?)
	AuthenticationFailed,				// Authentication failed

	// Interface user errors
	InvalidParameter = 400u,			// An invalid parameter was passed to this function
	DeviceNotRegistered,				// This device has not been registered, this function is not available

	// Internal application errors
	InvalidAPIRequest = 500u,			// The API did not recognize the request
	NoSupportedAuth,					// No authentication methods available that are supported by this application
	FailedDatabaseConnection,			// Failed to create SQLite connection
	FailedDatabaseCreation,				// Failed to create SQLite tables or communicate with SQLite
	KeychainUnavailable,				// OS Keychain not available on this system, Master Password cannot be stored
	UpdateConfigFailed,					// Failed to update Dashlane configuration in database
	InternalEncryptFailure,				// Encryption/Serialization failed
	InternalDecryptFailure,				// Decryption/Deserialization failed
	SSOAuthenticationUnsupported,		// SSO Authentication is unsupported
	DatabaseTransactionFailure,			// Failed to execute a transaction with the vault database
	UnkownRequestError,					// Indicates either an incorrect request to the web server or connection issue
	APIError_Timeout,					// The timestamp generated by the system is out-of-bounds (Possible Timeout)
	APIError_Authentication,			// The API did not recognize the request authentication header
	APIError_DeviceKey,					// The API did not recognize the device key for the specified user
	APIError_EndPoint,					// The API endpoint specified does not exist
};