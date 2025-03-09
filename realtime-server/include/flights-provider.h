#pragma once

#include "mysql-provider.h"

namespace RealtimeServer
{
    class FlightsProvider final : public Utils::MySqlProvider
    {
    public:
        explicit FlightsProvider(const std::string& dbHost,
                                 const int dbPort,
                                 const std::string& username,
                                 const std::string& password,
                                 const std::string& database);

        std::string getFlights();

        void populateFlights();

        ~FlightsProvider();

    private:
        bool areFlightsPopulated = false;
    };
}