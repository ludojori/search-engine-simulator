#include "provider.h"

#include <memory>

#include "mysql_connection.h"
#include "mysql_driver.h"
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "pair.h"
#include "user.h"
#include "pointer-wrapper.h"
#include "server-exceptions.h"

namespace ApiServer
{
    static const std::string usersRawStmt = "INSERT INTO users (name, password, type_id) VALUES (?,?,?)";
    static const std::string pairsRawStmt = "INSERT INTO pairs (origin, destination, is_one_way, is_roundtrip, f_carrier, m_carrier, o_carrier) VALUES (?,?,?,?,?,?,?)";

    Provider::Provider(const std::string& dbHost, const int dbPort, const std::string& username, const std::string& password, const std::string& database) :
                        _dbHost(dbHost), _dbPort(dbPort), _username(username), _password(password), _database(database)
    {
        _driver = get_driver_instance();
        _connection = _driver->connect(_dbHost, _username, _password);
        _connection->setSchema(_database);
    }

    void Provider::insertUser(const std::string& serializedUser)
    {
        try
        {
            const Utils::User user = Utils::parseUser(serializedUser);
            auto stmt = Utils::PointerWrapper(_connection->prepareStatement(usersRawStmt));
            stmt->setString(1, user.username);
            stmt->setString(2, user.password);
            stmt->setInt(3, static_cast<int>(user.type));
            stmt->execute();
        }
        catch(const std::exception& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }
    }

    void Provider::insertPair(const std::string& serializedPair)
    {
        try
        {
            const Utils::Pair pair = Utils::parsePair(serializedPair);
            auto stmt = Utils::PointerWrapper(_connection->prepareStatement(pairsRawStmt));
            stmt->setString(1, pair.origin);
            stmt->setString(2, pair.destination);
            stmt->setBoolean(3, pair.isOneWay);
            stmt->setBoolean(4, pair.isRoundtrip);
            stmt->setString(5, pair.fareCarrier);
            stmt->setString(6, pair.marketingCarrier);
            stmt->setString(7, pair.operatingCarrier);
            stmt->execute();
        }
        catch(const std::exception& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }
        
    }

    std::string Provider::getUsers()
    {
        try
        {       
            auto stmt = Utils::PointerWrapper(_connection->createStatement());
            auto result = Utils::PointerWrapper(stmt->executeQuery("SELECT * FROM users"));

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
            auto stmt = Utils::PointerWrapper(_connection->createStatement());
            auto result = Utils::PointerWrapper(stmt->executeQuery("SELECT * FROM pairs"));

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
            auto stmt = Utils::PointerWrapper(_connection->createStatement());
            auto result = Utils::PointerWrapper(
                stmt->executeQuery("SELECT * FROM pairs WHERE origin=" + origin + " AND destination=" + destination)
                );

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

        // Do NOT delete _driver* as required by library manual
    }
}
