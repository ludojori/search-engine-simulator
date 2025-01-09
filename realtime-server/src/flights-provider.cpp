#include "flights-provider.h"

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

    FlightsProvider::~FlightsProvider()
    {
        Utils::MySqlProvider::~MySqlProvider();
    }
}