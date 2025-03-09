#include "flights-provider.h"

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <random>

#include "pointer-wrapper.h"

namespace RealtimeServer
{
    FlightsProvider::FlightsProvider(const std::string& dbHost,
                                     const int dbPort,
                                     const std::string& username,
                                     const std::string& password,
                                     const std::string& database)
        : Utils::MySqlProvider(dbHost, dbPort, username, password, database)
    {
    }

    std::string FlightsProvider::getFlights()
    {
        auto stmt = createStatement();
        auto result = Utils::PointerWrapper(stmt->executeQuery("SELECT * FROM flights"));

        std::string resultStr = "[";

        while(result->next())
        {
            if(resultStr.size() > 1)
            {
                resultStr += ",";
            }

            // Utils::Flight flight {
            //     .origin = result->getString("origin"),
            //     .destination = result->getString("destination"),
            //     .departureTime = result->getString("departure_time"),
            //     .arrivalTime = result->getString("arrival_time"),
            //     .fareCarrier = result->getString("f_carrier"),
            //     .marketingCarrier = result->getString("m_carrier"),
            //     .operatingCarrier = result->getString("o_carrier")
            // };

            // resultStr += flight.serialize();
        }

        resultStr += "]";

        return resultStr;
    }

    void FlightsProvider::populateFlights()
    {
        if(areFlightsPopulated)
        {
            return;
        }

        double price = 0.0;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dis(100.0, 1000.0);

        auto stmt = createStatement();
        stmt->execute("INSERT INTO flights (origin, destination, departure_time, arrival_time, f_carrier, m_carrier, o_carrier) VALUES ('JFK', 'LAX', '2021-01-01 00:00:00', '2021-01-01 08:00:00', 'AA', 'AA', 'AA')");
        stmt->execute("INSERT INTO flights (origin, destination, departure_time, arrival_time, f_carrier, m_carrier, o_carrier) VALUES ('LAX', 'JFK', '2021-01-01 10:00:00', '2021-01-01 18:00:00', 'AA', 'AA', 'AA')");

        areFlightsPopulated = true;
    }

    FlightsProvider::~FlightsProvider()
    {
        Utils::MySqlProvider::~MySqlProvider();
    }
}