#include "flights-provider.h"

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <random>

#include "flight.h"
#include "pair.h"
#include "pointer-wrapper.h"

namespace RealtimeServer
{
    static double generateRandomPrice(double minPrice, double maxPrice)
    {
        std::random_device rd; // Seed for the random number engine
        std::mt19937 gen(rd()); // Mersenne Twister random number engine
        std::uniform_real_distribution<double> dis(minPrice, maxPrice); // Uniform distribution in the range [minPrice, maxPrice]

        return dis(gen);
    }

    static int generateRandomCabinClass()
    {
        std::random_device rd; // Seed for the random number engine
        std::mt19937 gen(rd()); // Mersenne Twister random number engine
        std::uniform_int_distribution<int> dis(0, 3); // Uniform distribution for cabin classes (0 to 3)

        return dis(gen);
    }

    Provider::Provider(const std::string& dbHost,
                       const int dbPort,
                       const std::string& username,
                       const std::string& password,
                       const std::string& database)
        : Utils::MySqlProvider(dbHost, dbPort, username, password, database)
    {
        populateFlightsTable();
    }

    std::string Provider::getFlights(const std::string& origin, const std::string& destination)
    {
        const bool setOrigin = !origin.empty();
        const bool setDestination = !destination.empty();

        std::string queryStr = "SELECT p.origin AS origin, "
                                      "p.destination AS destination, "
                                      "p.type AS type, "
                                      "p.f_carrier AS f_carrier, "
                                      "f.dep_datetime AS dep_datetime, "
                                      "f.arr_datetime AS arr_datetime, "
                                      "f.price AS price, "
                                      "f.currency AS currency, "
                                      "f.cabin AS cabin "
                               "FROM flights f JOIN pairs p ON f.pair_id = p.id";

        if(setOrigin || setDestination)
        {
            queryStr += " WHERE ";

            if(setOrigin)
            {
                queryStr += "origin='" + origin + "'";
                if(setDestination)
                {
                    queryStr += " AND ";
                }
            }

            if(setDestination)
            {
                queryStr += "destination='" + destination + "'";
            }
        }

        queryStr += ";";

        auto stmt = createStatement();
        auto result = Utils::PointerWrapper(stmt->executeQuery(queryStr));

        std::string resultStr = "[";

        while(result->next())
        {
            if(resultStr.size() > 1)
            {
                resultStr += ",";
            }

            Utils::Flight flight {
                .origin = result->getString("origin"),
                .destination = result->getString("destination"),
                .type = result->getBoolean("type") ? Utils::FlightType::Roundtrip : Utils::FlightType::OneWay,
                .departureTime = result->getString("dep_datetime"),
                .arrivalTime = result->getString("arr_datetime"),
                .fareCarrier = result->getString("f_carrier"),
                .price = result->getDouble("price"),
                .currency = result->getString("currency"),
                .cabin = static_cast<Utils::CabinType>(result->getInt("cabin"))
            };

            resultStr += flight.serialize();
        }

        resultStr += "]";

        return resultStr;
    }

    void Provider::populateFlightsTable()
    {
        if(areFlightsPopulated)
        {
            return;
        }

        try
        {
            {   // Check if flights table is already populated.
                auto stmt = createStatement();
                auto result = Utils::PointerWrapper(stmt->executeQuery("SELECT COUNT(*) AS count FROM flights"));
                if(result->next())
                {
                    if(result->getInt("count") > 0)
                    {
                        areFlightsPopulated = true;
                        return;
                    }
                }
            }

            std::vector<int> pairIds;

            {
                auto stmt = createStatement();
                auto result = Utils::PointerWrapper(stmt->executeQuery("SELECT id FROM pairs"));
                while(result->next())
                {
                    pairIds.push_back(result->getInt("id"));
                }
            }
    
            std::string insertQuery = "INSERT INTO flights (pair_id, dep_datetime, arr_datetime, price, currency, cabin) VALUES ";

            for(const auto& pairId : pairIds)
            {
                const std::string& departureDatetime = "2021-01-01 00:00:00";
                const std::string& arrivalDatetime = "2021-01-01 12:00:00";
                const double minPrice = 100.0;
                const double maxPrice = 1000.0;
    
                const double price = generateRandomPrice(minPrice, maxPrice);
    
                const std::string& currency = "USD";
                const int cabinClass = generateRandomCabinClass();
    
                insertQuery += "(" + std::to_string(pairId) + ", '" + departureDatetime + "', '" + arrivalDatetime + "', " + std::to_string(price) + ", '" + currency + "', " + std::to_string(cabinClass) + "),";
            }
    
            insertQuery.pop_back();
            insertQuery += ";";
    
            auto stmt = createStatement();
            stmt->execute(insertQuery);
    
            areFlightsPopulated = true;
        }
        catch(const sql::SQLException& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }
    }
}