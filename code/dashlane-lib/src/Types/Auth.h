#pragma once

namespace Dashlane
{

	enum class EAuthDeviceType
	{
		None,
		App,
		TeamDevice,
		UserDevice
	};

	struct IAuthenticationBase
	{
		IAuthenticationBase(EAuthDeviceType type) : type(type) {}

		const EAuthDeviceType type;
	};

	struct STeamDeviceAuthentication : public IAuthenticationBase
	{
		STeamDeviceAuthentication() : IAuthenticationBase(EAuthDeviceType::TeamDevice) {}

		std::string teamUuid;
		std::string accessKey;
		std::string secretKey;
		std::string appAccessKey;
		std::string appSecretKey;
	};

	struct SUserDeviceAuthentication : public IAuthenticationBase
	{
		SUserDeviceAuthentication() : IAuthenticationBase(EAuthDeviceType::UserDevice) {}

		std::string login;
		std::string accessKey;
		std::string secretKey;
		std::string appAccessKey;
		std::string appSecretKey;
	};

	struct SAppAuthentication : public IAuthenticationBase
	{
		SAppAuthentication() : IAuthenticationBase(EAuthDeviceType::App) {}

		std::string appAccessKey;
		std::string appSecretKey;
	};

	struct SNoneAuthentication : public IAuthenticationBase
	{
		SNoneAuthentication() : IAuthenticationBase(EAuthDeviceType::None) {}
	};

	enum class EAuthTicketType
	{
		SSO,
		MASTER_PASSWORD
	};

    NLOHMANN_JSON_SERIALIZE_ENUM(EAuthTicketType,
    {
        {EAuthTicketType::SSO, "sso"},
        {EAuthTicketType::MASTER_PASSWORD, "master_password"}
    });

	enum class EAuthMethod
	{
		SSO,
		U2F,
		EmailToken,
		Totp,
		DuoPush,
		DashlaneAuthenticator
	};

    NLOHMANN_JSON_SERIALIZE_ENUM(EAuthMethod,
    {
        {EAuthMethod::SSO, "sso"},
        {EAuthMethod::U2F, "u2f"},
        {EAuthMethod::EmailToken, "email_token"},
        {EAuthMethod::Totp, "totp"},
        {EAuthMethod::DuoPush, "duo_push"},
        {EAuthMethod::DashlaneAuthenticator, "dashlane_authenticator"}
    });
	
	enum class E2FAType
	{
		EmailToken,
		TotpDeviceRegistration,
		TotpLogin,
		SSO
	};

    NLOHMANN_JSON_SERIALIZE_ENUM(E2FAType,
    {
        {E2FAType::SSO, "sso"},
        {E2FAType::EmailToken, "email_token"},
        {E2FAType::TotpDeviceRegistration, "totp_device_registration"},
        {E2FAType::TotpLogin, "totp_login"}
    });

	enum class E2FAMigrationType
	{
		None,
		SSOMemberToAdmin,
		MPUserToSSOMember,
		SSOMemberToMPUser
	};

    NLOHMANN_JSON_SERIALIZE_ENUM(E2FAMigrationType,
    {
        {E2FAMigrationType::None, ""},
        {E2FAMigrationType::SSOMemberToAdmin, "sso_member_to_admin"},
        {E2FAMigrationType::MPUserToSSOMember, "mp_user_to_sso_member"},
        {E2FAMigrationType::SSOMemberToMPUser, "sso_member_to_mp_user"}
    });

	struct IAuthenticationMethodBase
	{

	protected:
		IAuthenticationMethodBase(EAuthMethod type) 
			: type(type) 
		{}

	public:
		EAuthMethod type;
	};

	struct SAuthenticationMethodSSO : public IAuthenticationMethodBase
	{
		SAuthenticationMethodSSO() 
			: IAuthenticationMethodBase(EAuthMethod::SSO)
		{};

		std::string serviceProviderUrl;
		E2FAMigrationType migration{ E2FAMigrationType::None };
		bool isNitroProvider{ false };
	};

	struct SAuthenticationMethodU2F : public IAuthenticationMethodBase
	{
		struct SChallenge
		{
			std::string challenge;
			std::string version;
			std::string appId;
			std::string keyHandle;
		};

		SAuthenticationMethodU2F() 
			: IAuthenticationMethodBase(EAuthMethod::U2F)
		{};

		std::vector<SChallenge> challenges;
	};

	struct SAuthenticationMethodEmailToken : public IAuthenticationMethodBase
	{
		SAuthenticationMethodEmailToken() 
			: IAuthenticationMethodBase(EAuthMethod::EmailToken) 
		{};
	};

	struct SAuthenticationMethodTotp : public IAuthenticationMethodBase
	{
		SAuthenticationMethodTotp() 
			: IAuthenticationMethodBase(EAuthMethod::Totp) 
		{};
	};

	struct SAuthenticationMethodDuoPush : public IAuthenticationMethodBase
	{
		SAuthenticationMethodDuoPush() 
			: IAuthenticationMethodBase(EAuthMethod::DuoPush) 
		{};
	};

	struct SAuthenticationMethodDashlaneAuthenticator : public IAuthenticationMethodBase
	{
		SAuthenticationMethodDashlaneAuthenticator() 
			: IAuthenticationMethodBase(EAuthMethod::DashlaneAuthenticator) 
		{};
	};

	struct SDeviceConfiguration
	{
		std::string accessKey;
		std::string secretKeyEncrypted;
		std::string masterPasswordEncrypted;
		bool        shouldNotSaveMasterPassword{ true };
		std::string localKeyEncrypted;
		std::string login;
		std::string version;
		bool        autoSync{ false };
		E2FAType    authenticationMode{ E2FAType::EmailToken };
		std::string serverKeyEncrypted;
	};

}