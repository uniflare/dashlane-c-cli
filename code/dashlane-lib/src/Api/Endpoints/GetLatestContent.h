#pragma once

#include <Dashlane.h>
#include <Api/ApiRequest.h>

namespace Dashlane
{

	struct SGetLatestContentResponse
	{
		struct STransaction
		{
			std::string identifier;
			uint32_t backupDate{ 0 };
		};

		struct SRevisionItem
		{
			std::string id;
			uint32_t revision{ 0 };
		};

		struct IKeyBase
		{
			enum class EType
			{
				Key,
				Record,
				None
			};

			IKeyBase(EType keyType) : keyType(keyType) {}
			EType keyType;
		};

		struct SKey : public IKeyBase
		{
			SKey() : IKeyBase(IKeyBase::EType::Key) {}
			std::string publicKey;
			std::string privateKey;
		};

		struct SRecord : public IKeyBase
		{
			SRecord() : IKeyBase(IKeyBase::EType::Record) {}
			std::string key;
		};

		struct SNone : public IKeyBase
		{
			SNone() : IKeyBase(IKeyBase::EType::None) {}
		};

		std::vector<std::unique_ptr<IRawTransaction>> transactions{};
		struct {
			std::vector<STransaction> transactions{};
			std::string content{};
		} fullBackup;
		uint32_t timestamp{};
		struct {
			std::vector<SRevisionItem> itemGroups{};
			std::vector<SRevisionItem> items{};
			std::vector<SRevisionItem> userGroups{};
		} sharing2;
		bool syncAllowed{ false };
		bool uploadEnabled{ false };
		std::map<std::string, std::map<std::string, uint64_t>> summary{};
		std::unique_ptr<IKeyBase> keys{};
	};

    inline void from_json(const nlohmann::ordered_json& j, SGetLatestContentResponse::STransaction& p)
    {
        j.at("identifier").get_to(p.identifier);

        if (j.contains("backupDate"))
        {
            j.at("backupDate").get_to(p.backupDate);
        }
    }

    inline void from_json(const nlohmann::ordered_json& j, SGetLatestContentResponse::SRevisionItem& p)
    {
        j.at("id").get_to(p.id);
        j.at("revision").get_to(p.revision);
    }

    inline void from_json(const nlohmann::ordered_json& j, std::unique_ptr<SGetLatestContentResponse::IKeyBase>& p)
    {
        if (j.contains("publicKey"))
        {
            p = std::make_unique<SGetLatestContentResponse::SKey>();
            auto& obj = static_cast<SGetLatestContentResponse::SKey&>(*p);
            j.at("publicKey").get_to(obj.publicKey);
            j.at("privateKey").get_to(obj.privateKey);
        }
        else
        {
            p = std::make_unique<SGetLatestContentResponse::SRecord>();
            auto& obj = static_cast<SGetLatestContentResponse::SRecord&>(*p);
            auto record = j.at(0).get<std::pair<std::string, std::string>>();
            obj.key = record.first;
        }

        throw(std::runtime_error("Error: Unknown Record"));
    }

    inline void from_json(const nlohmann::ordered_json& j, SGetLatestContentResponse& p)
    {
        j.at("data").at("transactions").get_to(p.transactions);

        if (j.at("data").at("fullBackup").contains("transactions"))
        {
            j.at("data").at("fullBackup").at("transactions").get_to(p.fullBackup.transactions);
        }

        if (j.at("data").at("fullBackup").contains("content"))
        {
            j.at("data").at("fullBackup").at("content").get_to(p.fullBackup.content);
        }

        j.at("data").at("timestamp").get_to(p.timestamp);
        j.at("data").at("sharing2").at("itemGroups").get_to(p.sharing2.itemGroups);
        j.at("data").at("sharing2").at("items").get_to(p.sharing2.items);
        j.at("data").at("sharing2").at("userGroups").get_to(p.sharing2.userGroups);
        j.at("data").at("syncAllowed").get_to(p.syncAllowed);
        j.at("data").at("uploadEnabled").get_to(p.uploadEnabled);
        j.at("data").at("summary").get_to(p.summary);

        if (j.at("data").contains("keys"))
        {
            j.at("data").at("keys").get_to(p.keys);
        }
    }

	inline EDashlaneError GetLatestContent(DashlaneContextInternal& context, uint64_t timestamp, Dashlane::SGetLatestContentResponse& response)
	{
		nlohmann::ordered_json result;
		EDashlaneError rc = Dashlane::RequestApi(
			context,
			"sync/GetLatestContent",
			result,
			{
				{"timestamp", timestamp},
				{"needsKeys", false},
				{"teamAdminGroups", false},
				{"transactions", std::vector<std::string>()}
			}
		);

		if (rc == EDashlaneError::NoError)
		{
			response = result.get<Dashlane::SGetLatestContentResponse>();
		}

		return rc;
	}

}