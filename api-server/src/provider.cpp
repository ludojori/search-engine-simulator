#include "provider.h"
#include "mysql_connection.h"
#include "mysql_driver.h"
#include <cppconn/statement.h>
#include "pair.h"
#include "user.h"
#include "server-exceptions.h"

namespace ApiServer
{
    Provider::Provider(const std::string& dbHost, const int dbPort, const std::string& username, const std::string& password, const std::string& database) :
                        _dbHost(dbHost), _dbPort(dbPort), _username(username), _password(password), _database(database)
    {
        _driver = get_driver_instance();
        _connection = _driver->connect(_dbHost, _username, _password);
        _connection->setSchema(_database);
    }

    void Provider::insertUser(const std::string&)
    {

    }

    void Provider::insertPair(const std::string&)
    {

    }

    std::string Provider::getUsers()
    {
        try
        {       
            auto stmt = _connection->createStatement();
            auto result = stmt->executeQuery("SELECT * FROM users");

            std::string resultStr = "[";

            while(result->next())
            {
                if(resultStr.size() > 1)
                {
                    resultStr += ",";
                }

                Utils::User user {
                    .username = result->getString("name"),
                    .password = result->getString("password"),
                    .type = static_cast<Utils::UserType>(result->getInt("type_id"))
                };

                const std::string serializedUser = user.serialize();
                resultStr += serializedUser.substr(0, serializedUser.size() - 1); // remove newline 
            }

            resultStr += "]";

            delete result;
            delete stmt;

            return resultStr;
        }
        catch(const std::exception& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }
    }

    std::string Provider::getPairs()
    {
        try
        {       
            auto stmt = _connection->createStatement();
            auto result = stmt->executeQuery("SELECT * FROM pairs");

            std::string resultStr = "[";

            while(result->next())
            {
                if(resultStr.size() > 1)
                {
                    resultStr += ",";
                }

                Utils::Pair pair {
                    .origin = result->getString("origin"),
                    .destination = result->getString("destination"),
                    .isOneWay = result->getBoolean("is_one_way"),
                    .isRoundtrip = result->getBoolean("is_roundtrip"),
                    .fareCarrier = result->getString("f_carrier"),
                    .marketingCarrier = result->getString("m_carrier"),
                    .operatingCarrier = result->getString("o_carrier")
                };

                const std::string serializedPair = pair.serialize();
                resultStr += serializedPair.substr(0, serializedPair.size() - 1); // remove newline
            }

            resultStr += "]";

            delete result;
            delete stmt;

            return resultStr;   
        }
        catch(const std::exception& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }
    }

    std::string Provider::getPair(const std::string& origin, const std::string& destination)
    {
        try
        {      
            auto stmt = _connection->createStatement();
            auto result = stmt->executeQuery("SELECT * FROM pairs WHERE origin=" + origin + " AND destination=" + destination);
            std::string resultStr = "";

            if(result->next())
            {
                Utils::Pair pair {
                    .origin = result->getString("origin"),
                    .destination = result->getString("destination"),
                    .isOneWay = result->getBoolean("is_one_way"),
                    .isRoundtrip = result->getBoolean("is_roundtrip"),
                    .fareCarrier = result->getString("f_carrier"),
                    .marketingCarrier = result->getString("m_carrier"),
                    .operatingCarrier = result->getString("o_carrier")
                };

                resultStr = pair.serialize();
            }

            delete result;
            delete stmt;

            return resultStr;
        }
        catch(const std::exception& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }
    }

    Provider::~Provider()
    {
        if(_connection)
        {
            delete _connection;
        }
    }
}
