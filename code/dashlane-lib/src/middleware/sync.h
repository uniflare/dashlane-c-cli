#pragma once

#include "../crypto/encrypt.h"
#include "../crypto/decrypt.h"
#include "../crypto/keychain_manager.h"
#include "../endpoints/getLatestContent.h"
#include "../types.h"
#include "../util.h"

#include <SQLiteCpp/SQLiteCpp.h>

namespace dlib
{
    inline void sync(
        const SQLite::Database& db,
        const Secrets& secrets,
        const DeviceConfiguration& deviceConfiguration)
    {
        uint32_t lastServerSyncTimestamp = 0;

        {
            SQLite::Statement stmt(db, "SELECT lastServerSyncTimestamp FROM syncUpdates WHERE login = ?");
            stmt.bind(1, secrets.login);
            if (stmt.executeStep())
            {
                lastServerSyncTimestamp = stmt.getColumn(0).getUInt();
            }
        }

        const auto latestContent = getLatestContent({
            secrets.login,
            lastServerSyncTimestamp,
            {
                secrets.accessKey,
                secrets.secretKey
            }
        });

        std::vector<std::vector<std::string>> values;
        bool masterPasswordValid = false;
        std::map<std::string, std::vector<uint8_t>> derivates;
        while (!masterPasswordValid)
        {
            masterPasswordValid = true;

            for (const auto& transaction : latestContent.transactions)
            {
                switch (transaction->action)
                {

                case EAction::BackupEdit:
                {
                    // TODO: Try/Catch for invalid master password
                    const auto& transac = static_cast<const BackupEditTransaction&>(*transaction);
                    std::vector<uint8_t> transactionContent = decrypt(transac.content, SymmetricKeyGetter_memoize{secrets, derivates});

                    const std::string encryptedTransactionContent = encryptAES(secrets.localKey, transactionContent);

                    values.emplace_back(
                        std::vector<std::string>{
                            secrets.login,
                            transac.identifier,
                            transac.type,
                            nlohmann::ordered_json(transac.action),
                            encryptedTransactionContent
                        }
                    );
                } break;

                case EAction::BackupRemove:
                {
                    const auto& transac = static_cast<const BackupRemoveTransaction&>(*transaction);
                    values.emplace_back(
                        std::vector<std::string>{
                            secrets.login,
                            transac.identifier,
                            transac.type,
                            nlohmann::ordered_json(transac.action),
                            ""
                        }
                    );
                } break;

                }
            }
        }

        for (const auto& value : values)
        {
            SQLite::Statement stmt(db, "REPLACE INTO transactions (login, identifier, type, action, content) VALUES (?, ?, ?, ?, ?)");
            stmt.bind(1, value.at(0));
            stmt.bind(2, value.at(1));
            stmt.bind(3, value.at(2));
            stmt.bind(4, value.at(3));
            stmt.bind(5, value.at(4));
            stmt.exec();
        }

        {
            SQLite::Statement stmt(db, "REPLACE INTO syncUpdates (login, lastServerSyncTimestamp, lastClientSyncTimestamp) VALUES(?, ?, ?)");
            stmt.bind(1, secrets.login);
            stmt.bind(2, latestContent.timestamp);
            stmt.bind(3, static_cast<uint32_t>(util::get_unix_timestamp()));
            stmt.exec();
        }

        std::cout << "Sync complete" << std::endl;
    }
}