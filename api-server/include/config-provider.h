#pragma once

#include "mysql-provider.h"

namespace Utils
{
    class User;
    class Pair;
}

namespace ApiServer
{
    class ConfigProvider final : public Utils::MySqlProvider
    {
    public:
        explicit ConfigProvider(const std::string& dbHost,
                                const int dbPort,
                                const std::string& username,
                                const std::string& password,
                                const std::string& database);

        void insertUser(const Utils::User& user);
        void insertPair(const Utils::Pair& pair);
        std::string getUsers();
        std::string getPairs();
        std::string getPair(const std::string& origin, const std::string& destination);

    private:
        void authorizeOperation(Utils::OperationType opType);
        Utils::PermissionLevel acquirePermissionLevel();
    };
}