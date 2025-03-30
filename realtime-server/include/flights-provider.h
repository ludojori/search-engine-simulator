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

        void populateFlightsTable();
        std::string getFlights(const std::string& origin, const std::string& destination);
        
        ~FlightsProvider();
        
    private:
        double generateRandomPrice(double minPrice, double maxPrice);

        bool areFlightsPopulated = false;
    };
}