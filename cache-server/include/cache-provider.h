#pragma once

#include "mysql-provider.h"

namespace CacheServer
{
    class CacheProvider final : public Utils::MySqlProvider
    {
    public:
        explicit CacheProvider(const std::string& dbHost,
                               const int dbPort,
                               const std::string& username,
                               const std::string& password,
                               const std::string& database);

        std::string getFlights(const std::string& origin, const std::string& destination);

        ~CacheProvider();

    private:
    };
}