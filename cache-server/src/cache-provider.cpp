#include "cache-provider.h"

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "flight.h"
#include "pointer-wrapper.h"

namespace CacheServer
{
    Provider::Provider(const std::string& dbHost,
                       const int dbPort,
                       const std::string& username,
                       const std::string& password,
                       const std::string& database)
        : Utils::MySqlProvider(dbHost, dbPort, username, password, database) {}

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

        std::cout << "[DEBUG] Executing query " << queryStr << std::endl;

        try
        {
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
        catch(const std::exception& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }
    }
}