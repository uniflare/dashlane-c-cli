#pragma once

#include "../crypto/encrypt.h"
#include "../crypto/keychain_manager.h"
#include "../types.h"
#include "../util.h"

#include <SQLiteCpp/SQLiteCpp.h>

namespace dlib
{
    struct ConfigureSaveMasterPassword
    {
        SQLite::Database db;
        Secrets secrets;
        bool shouldNotSaveMasterPassword;
    };

    struct ConfigureDisableAutoSync
    {
        SQLite::Database db;
        Secrets secrets;
        bool disableAutoSync;
    };

    inline void configureSaveMasterPassword(const ConfigureSaveMasterPassword& params)
    {
        bool shouldNotSaveMasterPassword = params.shouldNotSaveMasterPassword;
        std::vector<uint8_t> masterPasswordEncrypted;
        if (shouldNotSaveMasterPassword)
        {
            deleteLocalKey(params.secrets.login);
        }
        else
        {
            masterPasswordEncrypted = encryptAES(params.secrets.localKey, util::fromString<std::vector<uint8_t>>(params.secrets.masterPassword));

            setLocalKey(params.secrets.login, params.secrets.localKey, [&shouldNotSaveMasterPassword](const keychain::Error& err) {
                std::cout << "Warning: Keychain storage is disabled - " << err.message << std::endl;
                shouldNotSaveMasterPassword = true;
            });
        }

        SQLite::Statement query(params.db, "UPDATE device SET masterPasswordEncrypted = ?, shouldNotSaveMasterPassword = ? WHERE login = ?");
        query.bind(1, static_cast<const void*>(masterPasswordEncrypted.data()), masterPasswordEncrypted.size());
        query.bind(2, shouldNotSaveMasterPassword ? 1 : 0);
        query.bind(3, params.secrets.login);
        query.exec();
    }

    inline void configureDisableAutoSync(const ConfigureDisableAutoSync& params)
    {
        SQLite::Statement query(params.db, "UPDATE device SET autoSync = ? WHERE login = ?");
        query.bind(1, params.secrets.login);
        query.bind(2, params.disableAutoSync ? 1 : 0);
        query.exec();
    }
}