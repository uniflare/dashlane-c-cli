#pragma once

#include <dashlane/Dashlane.h>
#include "Utility/EnumClass.h"

namespace Dashlane
{

    enum class ETransactionAction
    {
        Unknown,
        BackupEdit,
        BackupRemove
    };

    enum class ERawTransactionType : size_t
	{
		Authentifiant           = 1 << 0,
        BankStatement           = 1 << 1,
		CategoryAuth            = 1 << 2,
		Category_SecureNote     = 1 << 3,
		DriverLicense           = 1 << 4,
		Email                   = 1 << 5,
		Identity                = 1 << 6,
		PaymentMeansCreditCard  = 1 << 7,
		SecureNote              = 1 << 8,
		Settings                = 1 << 9,

        Count = 10
	};

	DECLARE_ENUM_BITMASK(ERawTransactionType, ERawTransactionType::Count);

    struct IRawTransaction
    {
		virtual ETransactionAction GetAction() const = 0;
		virtual std::string GetActionName() const = 0;
		virtual uint32_t GetBackupDate() const = 0;
		virtual std::string GetIdentifier() const = 0;
		virtual uint32_t GetTime() const = 0;
		virtual std::string GetContent() const = 0;
		virtual std::string GetType() const = 0;
    };

    struct SRawTransactionBackupEdit : public IRawTransaction
    {
		// IRawTransaction
        virtual ETransactionAction GetAction() const override { return ETransactionAction::BackupEdit; }
        virtual std::string GetActionName() const override { return nlohmann::ordered_json(GetAction()); }
        virtual uint32_t GetBackupDate() const override { return backupDate; }
		virtual std::string GetIdentifier() const override { return identifier; }
		virtual uint32_t GetTime() const override { return time; }
		virtual std::string GetContent() const override { return content; }
		virtual std::string GetType() const override { return type; }
        // ~IRawTransaction

        uint32_t    backupDate{0};
        std::string identifier;
        uint32_t    time{0};
        std::string content;
        std::string type;
    };

    struct SRawTransactionBackupRemove : public IRawTransaction
	{
		// IRawTransaction
		virtual ETransactionAction GetAction() const override { return ETransactionAction::BackupRemove; }
		virtual std::string GetActionName() const override { return nlohmann::ordered_json(GetAction()); }
		virtual uint32_t GetBackupDate() const override { return backupDate; }
		virtual std::string GetIdentifier() const override { return identifier; }
		virtual uint32_t GetTime() const override { return time; }
        virtual std::string GetContent() const override { return ""; }
		virtual std::string GetType() const override { return type; }
		// ~IRawTransaction

        uint32_t    backupDate{0};
        std::string identifier;
        uint32_t    time{0};
        std::string type;
	};

	struct DASHLANE_API ITransactionBase
	{
		ITransactionBase(ETransactionType type)
			: type(type)
		{}

		ETransactionType type;
	};

	struct DASHLANE_API STransactionUnknown : public ITransactionBase
	{
		STransactionUnknown()
			: ITransactionBase(ETransactionType::Unknown)
		{}
	};

	struct DASHLANE_API STransactionAuthentifiant : public ITransactionBase
	{
		STransactionAuthentifiant()
			: ITransactionBase(ETransactionType::Authentifiant)
		{}

		enum class EStatus
		{
			ACCOUNT_NOT_VERIFIED,
			ACCOUNT_VERIFIED,
			ACCOUNT_INVALID
		};

		std::string title;
		std::string email;
		std::string login;
		std::string password;
		std::string url;
		std::string secondaryLogin;
		std::string category;
		std::string note;
		std::string lastBackupTime; // timestamp
		bool        autoProtected{ false };
		bool        autoLogin{ false };
		bool        subdomainOnly{ false };
		bool        useFixedUrl{ false };
		std::string otpSecret;
		std::string appMetaData; // info about linked mobile applications
		EStatus     status{ EStatus::ACCOUNT_INVALID };
		std::string numberUse; // number
		std::string lastUse; // timestamp
		std::string strength; // number between 0 and 100
		std::string modificationDatetime; // timestamp
		bool        checked{ false };
		std::string id;
		std::string anonId;
		std::string localeFormat; // either UNIVERSAL or a country code
	};

	struct DASHLANE_API STransactionSecureNote : public ITransactionBase
	{
		STransactionSecureNote()
			: ITransactionBase(ETransactionType::SecureNote)
		{}

		std::string anonId;
		std::string category;
		std::string content;
		std::string creationDate;
		std::string creationDateTime;
		std::string id;
		std::string lastBackupTime;
		bool        secured{ false }; // either true or false
		std::string spaceId;
		std::string title;
		std::string updateDate;
		std::string localeFormat; // either UNIVERSAL or a country code
		std::string type;
		std::string sharedObject;
		std::string userModificationDatetime;
	};

    NLOHMANN_JSON_SERIALIZE_ENUM(STransactionAuthentifiant::EStatus,
    {
        {STransactionAuthentifiant::EStatus::ACCOUNT_NOT_VERIFIED, "ACCOUNT_NOT_VERIFIED"},
        {STransactionAuthentifiant::EStatus::ACCOUNT_VERIFIED, "ACCOUNT_VERIFIED"},
        {STransactionAuthentifiant::EStatus::ACCOUNT_INVALID, "ACCOUNT_INVALID"}
    });

    NLOHMANN_JSON_SERIALIZE_ENUM(ETransactionAction,
    {
        {ETransactionAction::Unknown, ""},
        {ETransactionAction::BackupEdit, "BACKUP_EDIT"},
        {ETransactionAction::BackupRemove, "BACKUP_REMOVE"}
    });

    NLOHMANN_JSON_SERIALIZE_ENUM(ERawTransactionType,
    {
		{ERawTransactionType::Authentifiant,            "AUTHENTIFIANT"},
		{ERawTransactionType::BankStatement,            "BANKSTATEMENT"},
		{ERawTransactionType::CategoryAuth,             "AUTH_CATEGORY"},
		{ERawTransactionType::Category_SecureNote,      "SECURENOTE_CATEGORY"},
		{ERawTransactionType::DriverLicense,            "DRIVERLICENCE"},
		{ERawTransactionType::Email,                    "EMAIL"},
		{ERawTransactionType::Identity,                 "IDENTITY"},
		{ERawTransactionType::PaymentMeansCreditCard,   "PAYMENTMEANS_CREDITCARD"},
		{ERawTransactionType::SecureNote,               "SECURENOTE"},
		{ERawTransactionType::Settings,                 "SETTINGS"}
    });

    inline void from_json(const nlohmann::ordered_json& j, STransactionAuthentifiant& cred)
    {
        j.at("Title").get_to(cred.title);
        j.at("Email").get_to(cred.email);
        j.at("Login").get_to(cred.login);
        j.at("Password").get_to(cred.password);
        j.at("Url").get_to(cred.url);
        j.at("SecondaryLogin").get_to(cred.secondaryLogin);
        j.at("Category").get_to(cred.category);
        j.at("Note").get_to(cred.note);
        j.at("LastBackupTime").get_to(cred.lastBackupTime);
        cred.autoProtected = j.at("AutoProtected").get<std::string>() == "false" ? false : true;
        cred.autoLogin = j.at("AutoLogin").get<std::string>() == "false" ? false : true;
        cred.subdomainOnly = j.at("SubdomainOnly").get<std::string>() == "false" ? false : true;
        cred.useFixedUrl = j.at("UseFixedUrl").get<std::string>() == "false" ? false : true;
        j.at("OtpSecret").get_to(cred.otpSecret);
        if (j.contains("AppMetaData"))
        {
            j.at("AppMetaData").get_to(cred.appMetaData);
        }
		j.at("Status").get_to(cred.status);
		if (j.contains("NumberUse"))
		{
			j.at("NumberUse").get_to(cred.numberUse);
		}
		if (j.contains("LastUse"))
		{
			j.at("LastUse").get_to(cred.lastUse);
		}
        j.at("Strength").get_to(cred.strength);
        j.at("ModificationDatetime").get_to(cred.modificationDatetime);
        cred.checked = j.at("Checked").get<std::string>() == "false" ? false : true;
        j.at("Id").get_to(cred.id);
        j.at("AnonId").get_to(cred.anonId);
        j.at("LocaleFormat").get_to(cred.localeFormat);
	}

	inline void from_json(const nlohmann::ordered_json& j, STransactionSecureNote& cred)
	{
		j.at("Id").get_to(cred.id);
		j.at("AnonId").get_to(cred.anonId);
		j.at("CreationDatetime").get_to(cred.creationDateTime);
		j.at("LocaleFormat").get_to(cred.localeFormat);
		j.at("SpaceId").get_to(cred.spaceId);
		j.at("UserModificationDatetime").get_to(cred.userModificationDatetime);
		j.at("LastBackupTime").get_to(cred.lastBackupTime);
		j.at("Title").get_to(cred.title);
		j.at("Content").get_to(cred.content);
		j.at("Category").get_to(cred.category);
		j.at("Secured").get_to(cred.secured);
		j.at("Type").get_to(cred.type);

        if (j.contains("UpdateDate"))
		{
			j.at("UpdateDate").get_to(cred.updateDate);
        }
	}

    inline void from_json(const nlohmann::ordered_json& j, std::unique_ptr<IRawTransaction>& p)
    {
        switch (j.at("action").get<ETransactionAction>())
        {

        case ETransactionAction::BackupEdit:
        {
            p = std::make_unique<SRawTransactionBackupEdit>();
            auto& obj = static_cast<SRawTransactionBackupEdit&>(*p);

            j.at("backupDate").get_to(obj.backupDate);
            j.at("identifier").get_to(obj.identifier);
            j.at("time").get_to(obj.time);
            j.at("content").get_to(obj.content);
            j.at("type").get_to(obj.type);
        } break;

        case ETransactionAction::BackupRemove:
        {
            p = std::make_unique<SRawTransactionBackupRemove>();
            auto& obj = static_cast<SRawTransactionBackupRemove&>(*p);
            j.at("backupDate").get_to(obj.backupDate);
            j.at("identifier").get_to(obj.identifier);
            j.at("time").get_to(obj.time);
            j.at("type").get_to(obj.type);
        } break;

        }
	}

	inline void to_json(nlohmann::ordered_json& j, const std::unique_ptr<IRawTransaction>& p)
	{
		switch (p->GetAction())
		{

		case ETransactionAction::BackupEdit:
		{
			auto& obj = static_cast<const SRawTransactionBackupEdit&>(*p);
            j = {
				{"backupDate", obj.backupDate},
				{"identifier",obj.identifier},
				{"time",obj.time},
				{"content",obj.content},
				{"type",obj.type}
            };
		} break;

		case ETransactionAction::BackupRemove:
		{
			auto& obj = static_cast<const SRawTransactionBackupRemove&>(*p);
			j = {
				{"backupDate", obj.backupDate},
				{"identifier",obj.identifier},
				{"time",obj.time},
				{"type",obj.type}
			};
		} break;

		}
	}

}