#pragma once

#include "../crypto/keychain_manager.h"
#include "../types.h"

#include <SQLiteCpp/SQLiteCpp.h>

namespace dlib
{
    void reset(const SQLite::Database& db, const std::string& login)
    {
        SQLite::Statement(db, "DROP TABLE IF EXISTS syncUpdates").exec();
        SQLite::Statement(db, "DROP TABLE IF EXISTS transactions").exec();
        SQLite::Statement(db, "DROP TABLE IF EXISTS device").exec();

        if (!login.empty())
        {
            deleteLocalKey(login);
        }

        std::cout << "Reset complete" << std::endl;
    }
}