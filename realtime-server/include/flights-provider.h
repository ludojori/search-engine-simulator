#pragma once

#include "mysql-provider.h"

namespace RealtimeServer
{
    class Provider final : public Utils::MySqlProvider
    {
    public:
        explicit Provider(const std::string& dbHost,
                          const int dbPort,
                          const std::string& username,
                          const std::string& password,
                          const std::string& database);

        void populateFlightsTable();
        std::string getFlights(const std::string& origin, const std::string& destination);
        
    private:
    };
}