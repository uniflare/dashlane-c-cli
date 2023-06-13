#include "StdAfx.h"
#include "Database.h"
#include "Dashlane.h"
#include "Utility/Filesystem.h"
#include "Utility/Strings.h"
#include "Utility/Time.h"

#include <SQLiteCpp/SQLiteCpp.h>

#include <array>

namespace Dashlane
{

	static constexpr char APP_FOLDER[] = "dashlane-c-cli";

	CDatabase::CDatabase(const std::filesystem::path& dbPath)
		: m_dbPath(dbPath)
		, m_pDatabase(nullptr)
	{
		if (m_dbPath.empty())
		{
			const std::filesystem::path folder = Utility::GetApplicationDataFolder() / APP_FOLDER;
			if (!std::filesystem::exists(folder))
				std::filesystem::create_directories(folder);

			m_dbPath = folder / "userdata.db";
		}
	}

	CDatabase::~CDatabase() {}

	bool CDatabase::Connect()
	{
		m_pDatabase = std::make_unique<SQLite::Database>(m_dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
		return m_pDatabase != nullptr;
	}

	bool CDatabase::Prepare()
	{
		if (m_pDatabase)
		{
			SQLite::Statement(*m_pDatabase,
				"CREATE TABLE IF NOT EXISTS syncUpdates( " \
				"login VARCHAR(255) PRIMARY KEY, " \
				"lastServerSyncTimestamp INT, " \
				"lastClientSyncTimestamp INT " \
				");"
			).exec();

			SQLite::Statement(*m_pDatabase,
				"CREATE TABLE IF NOT EXISTS transactions ( " \
				"login VARCHAR(255), " \
				"identifier VARCHAR(255), " \
				"type VARCHAR(255) NOT NULL, " \
				"action VARCHAR(255) NOT NULL, " \
				"content BLOB, " \
				"PRIMARY KEY (login, identifier) " \
				");"
			).exec();

			SQLite::Statement(*m_pDatabase,
				"CREATE TABLE IF NOT EXISTS device ( " \
				"login VARCHAR(255) PRIMARY KEY, " \
				"version VARCHAR(255) NOT NULL, " \
				"accessKey VARCHAR(255) NOT NULL, " \
				"secretKeyEncrypted VARCHAR(255) NOT NULL, " \
				"masterPasswordEncrypted VARCHAR(255), " \
				"shouldNotSaveMasterPassword BIT NOT NULL, " \
				"localKeyEncrypted VARCHAR(255) NOT NULL, " \
				"autoSync BIT NOT NULL, " \
				"authenticationMode VARCHAR(255), " \
				"serverKeyEncrypted VARCHAR(255) " \
				");"
			).exec();

			return true;
		}

		return false;
	}

	void CDatabase::GetRegisteredUsers(std::vector<std::string>& users) const
	{
		SQLite::Statement stmt(*m_pDatabase, "SELECT login FROM device");

		while (stmt.executeStep())
			users.emplace_back(stmt.getColumn(0).getString());
	}

	void CDatabase::RemoveUserData(const DashlaneContextInternal& context)
	{
		std::array<const char*, 3> tables
		{
			"device",
			"transactions",
			"syncUpdates"
		};

		for (const auto table : tables)
		{
			SQLite::Statement stmt(*m_pDatabase, std::format("DELETE FROM {} WHERE login = ?", table));
			stmt.bindNoCopy(1, context.login);
			stmt.exec();
		}
	}

	void CDatabase::Drop()
	{
		if (m_pDatabase)
		{
			m_pDatabase->exec(
				"DROP TABLE IF EXISTS syncUpdates;" \
				"DROP TABLE IF EXISTS transactions;" \
				"DROP TABLE IF EXISTS device"
			);
		}
	}

	void CDatabase::Disconnect()
	{
		m_pDatabase.reset();
	}

	bool CDatabase::GetDeviceConfiguration(const DashlaneContextInternal& context, SDeviceConfiguration& config) const
	{
		bool success = false;

		if (m_pDatabase)
		{
			SQLite::Statement stmt(*m_pDatabase, "SELECT * FROM device WHERE login = ? LIMIT 1");
			stmt.bindNoCopy(1, context.login);

			if (stmt.executeStep())
			{
				if (stmt.hasRow())
				{
					config =
					{
						stmt.getColumn("accessKey").getString(),
						stmt.getColumn("secretKeyEncrypted").getString(),
						stmt.getColumn("masterPasswordEncrypted").getString(),
						static_cast<bool>(stmt.getColumn("shouldNotSaveMasterPassword").getInt()),
						stmt.getColumn("localKeyEncrypted").getString(),
						stmt.getColumn("login").getString(),
						stmt.getColumn("version").getString(),
						static_cast<bool>(stmt.getColumn("autoSync").getInt()),
						static_cast<E2FAType>(stmt.getColumn("authenticationMode").getInt()),
						stmt.getColumn("serverKeyEncrypted").getString()
					};

					success = true;
				}
			}

			// Default device configuration
			if (!success)
				config = SDeviceConfiguration();
		}

		return success;
	}

	bool CDatabase::SetDeviceConfiguration(const SDeviceConfiguration& config)
	{
		SQLite::Statement stmt(*m_pDatabase, "REPLACE INTO device VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

		stmt.bindNoCopy(1, config.login);
		stmt.bindNoCopy(2, config.version);
		stmt.bindNoCopy(3, config.accessKey);
		stmt.bindNoCopy(4, config.secretKeyEncrypted);

		if (config.shouldNotSaveMasterPassword)
			stmt.bindNoCopy(5, "");
		else
			stmt.bindNoCopy(5, config.masterPasswordEncrypted);

		stmt.bind(6, config.shouldNotSaveMasterPassword);
		stmt.bindNoCopy(7, config.localKeyEncrypted);
		stmt.bind(8, config.autoSync);
		stmt.bind(9, std::underlying_type_t<E2FAType>(config.authenticationMode));
		stmt.bindNoCopy(10, config.serverKeyEncrypted);

		stmt.exec();

		return true;
	}

	uint64_t CDatabase::GetLastSyncTime(DashlaneContextInternal& context) const
	{
		uint64_t time = 0;

		SQLite::Statement stmt(*m_pDatabase, "SELECT lastClientSyncTimestamp FROM syncUpdates WHERE login = ?");
		stmt.bindNoCopy(1, context.login);

		if (stmt.executeStep())
			time = stmt.getColumn(0).getUInt();

		return time;
	}

	bool CDatabase::UpdateLastSyncTime(DashlaneContextInternal& context, uint32_t lastServerSyncTime)
	{
		SQLite::Statement stmt(*m_pDatabase, "REPLACE INTO syncUpdates (login, lastServerSyncTimestamp, lastClientSyncTimestamp) VALUES(?, ?, ?)");
		stmt.bindNoCopy(1, context.login);
		stmt.bind(2, lastServerSyncTime);
		stmt.bind(3, static_cast<uint32_t>(Utility::GetUnixTimestamp()));
		return stmt.exec() > 0;
	}

	EDashlaneError CDatabase::GetTransactions(const DashlaneContextInternal& context, const bitmask<ERawTransactionType> types, std::vector<SRawTransactionBackupEdit>& transactions) const
	{
		// Build filter query
		std::string typeQuery;
		for (const auto& type : types)
		{
			if (typeQuery.empty())
				typeQuery = " AND (`type` = ?";
			else
				typeQuery += " OR `type` = ?";
		}
		if (!typeQuery.empty()) typeQuery += ")";

		SQLite::Statement stmt(*m_pDatabase, std::format("SELECT * FROM transactions WHERE login = ? AND action = 'BACKUP_EDIT'{}", typeQuery));
		int bindPos = 1;
		stmt.bindNoCopy(bindPos++, context.login);

		for (const auto& type : types)
			stmt.bind(bindPos++, std::string(nlohmann::ordered_json(type.value)));

		while (stmt.executeStep())
		{
			SRawTransactionBackupEdit transaction;
			transaction.identifier = stmt.getColumn("identifier").getString();
			transaction.content = stmt.getColumn("content").getString();
			transaction.type = stmt.getColumn("type").getString();
			transactions.emplace_back(std::move(transaction));
		}

		return EDashlaneError::NoError;
	}

	bool CDatabase::AddTransactionData(const STransactionRow& row)
	{
		SQLite::Statement stmt(*m_pDatabase, "REPLACE INTO transactions (login, identifier, type, action, content) VALUES (?, ?, ?, ?, ?)");
		stmt.bindNoCopy(1, row.login);
		stmt.bindNoCopy(2, row.identifier);
		stmt.bindNoCopy(3, row.type);
		stmt.bindNoCopy(4, row.action);
		stmt.bindNoCopy(5, row.content);
		
		return stmt.exec() > 0;
	}

	bool CDatabase::AddMultipleTransactionData(const std::vector<STransactionRow>& rows)
	{
		std::string query = "REPLACE INTO transactions(login, identifier, type, action, content) VALUES (?, ?, ?, ?, ?)";
		for (int i = 1; i < rows.size(); i++)
			query += ", (?, ?, ?, ?, ?)";

		SQLite::Statement stmt(*m_pDatabase, query);

		for (int i = 0; i < rows.size(); i++)
		{
			const size_t offset = 5 * i;
			stmt.bindNoCopy(offset + 1, rows[i].login);
			stmt.bindNoCopy(offset + 2, rows[i].identifier);
			stmt.bindNoCopy(offset + 3, rows[i].type);
			stmt.bindNoCopy(offset + 4, rows[i].action);
			stmt.bindNoCopy(offset + 5, rows[i].content);
		}

		return stmt.exec() > 0;
	}

}