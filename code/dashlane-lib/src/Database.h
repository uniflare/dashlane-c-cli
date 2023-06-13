#pragma once

#include "Types/Auth.h"
#include "Types/Transactions.h"

// Forward Decls.
namespace SQLite
{
	class Database;
}

namespace Dashlane
{

	struct DashlaneContextInternal;

	struct STransactionRow
	{
		STransactionRow(const std::string& login, const std::string& identifier, 
			const std::string& type, const std::string& action, const std::string& content)
			: login(login)
			, identifier(identifier)
			, type(type)
			, action(action)
			, content(content)
		{}

		std::string login;
		std::string identifier;
		std::string type;
		std::string action;
		std::string content;
	};

	class CDatabase
	{

	public:

		CDatabase(const std::filesystem::path& dbPath = "");
		~CDatabase();

		bool Connect();
		bool Prepare();
		void Disconnect();
		void Drop();

		void GetRegisteredUsers(std::vector<std::string>& users) const;
		void RemoveUserData(const DashlaneContextInternal& context);

		bool GetDeviceConfiguration(const DashlaneContextInternal& context, SDeviceConfiguration& config) const;
		bool SetDeviceConfiguration(const SDeviceConfiguration& config);

		uint64_t GetLastSyncTime(DashlaneContextInternal& pContext) const;
		bool UpdateLastSyncTime(DashlaneContextInternal& pContext, uint32_t lastServerSyncTime);

		bool AddTransactionData(const STransactionRow& row);
		bool AddMultipleTransactionData(const std::vector<STransactionRow>& rows);
		EDashlaneError GetTransactions(const DashlaneContextInternal& context, bitmask<ERawTransactionType> types, 
			std::vector<SRawTransactionBackupEdit>& transactions) const;

	private:

		std::filesystem::path m_dbPath;
		std::unique_ptr<SQLite::Database> m_pDatabase;

	};

}