#include "config-provider.h"

#include <memory>

#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>

#include "pair.h"
#include "user.h"
#include "pointer-wrapper.h"
#include "server-exceptions.h"

namespace ApiServer
{
    static const std::string usersRawStmt = "INSERT INTO users (name, password, type_id) VALUES (?,?,?)";
    static const std::string pairsRawStmt = "INSERT INTO pairs (origin, destination, type, f_carrier) VALUES (?,?,?,?,?)";

    ConfigProvider::ConfigProvider(const std::string& dbHost, const int dbPort, const std::string& username, const std::string& password, const std::string& database) :
        Utils::MySqlProvider(dbHost, dbPort, username, password, database) {}

    void ConfigProvider::insertUserSafe(const Utils::User& user)
    {
        try
        {
            auto stmt = prepareStatement(usersRawStmt);
            stmt->setString(1, user.username);
            stmt->setString(2, user.password);
            stmt->setInt(3, static_cast<int>(user.type));
            stmt->execute();
        }
        catch(const sql::SQLException& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }
    }

    void ConfigProvider::insertUserUnsafe(const Utils::User& user)
    {
        const std::string queryStr = "INSERT INTO users (name, password, type_id) VALUES ('" + user.username + "','" + user.password + "'," + std::to_string(static_cast<int>(user.type)) + ")";
        
        try
        {
            auto stmt = createStatement();
            stmt->execute(queryStr);
        }
        catch(const sql::SQLException& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }
    }

    void ConfigProvider::insertPairSafe(const Utils::Pair& pair)
    {
        const std::string serializedPair = getPair(pair.origin, pair.destination);
        if(!serializedPair.empty())
        {
            throw Utils::HttpStateConflict("Pair already exists.");
        }

        try
        {
            auto stmt = prepareStatement(pairsRawStmt);
            stmt->setString(1, pair.origin);
            stmt->setString(2, pair.destination);
            stmt->setBoolean(3, pair.type);
            stmt->setString(4, pair.fareCarrier);
            stmt->execute();
        }
        catch(const sql::SQLException& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }   
    }

    std::string ConfigProvider::getUsers()
    {
        try
        {       
            auto stmt = createStatement();
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
        catch(const sql::SQLException& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }
    }

    std::string ConfigProvider::getPairs()
    {
        try
        {       
            auto stmt = createStatement();
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
                    .type = result->getBoolean("type"),
                    .fareCarrier = result->getString("f_carrier")
                };

                const std::string serializedPair = pair.serialize();
                resultStr += serializedPair.substr(0, serializedPair.size() - 1); // remove newline
            }

            resultStr += "]";

            return resultStr;   
        }
        catch(const sql::SQLException& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }
    }

    std::string ConfigProvider::getPair(const std::string& origin, const std::string& destination)
    {
        std::string resultStr = "";

        try
        {      
            auto stmt = createStatement();
            auto result = Utils::PointerWrapper(
                stmt->executeQuery("SELECT * FROM pairs WHERE origin='" + origin + "' AND destination='" + destination + "'")
            );

            if(result->next())
            {
                Utils::Pair pair {
                    .origin = result->getString("origin"),
                    .destination = result->getString("destination"),
                    .type = result->getBoolean("type"),
                    .fareCarrier = result->getString("f_carrier")
                };

                resultStr = pair.serialize();
            }
        }
        catch(const sql::SQLException& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }

        if(resultStr.empty())
        {
            throw Utils::HttpNotFound("Pair not found.");
        }

        return resultStr;
    }

    std::string ConfigProvider::getPairUnsafe(const std::string& origin, const std::string& destination)
    {
        const std::string queryStr = "SELECT * FROM pairs WHERE origin='" + origin + "' AND destination='" + destination + "'";
        std::string resultStr = "";

        try
        {      
            auto stmt = createStatement();
            auto result = Utils::PointerWrapper(stmt->executeQuery(queryStr));

            while(result->next())
            {
                Utils::Pair pair {
                    .origin = result->getString("origin"),
                    .destination = result->getString("destination"),
                    .type = result->getBoolean("type"),
                    .fareCarrier = result->getString("f_carrier")
                };

                resultStr += pair.serialize();
            }
        }
        catch(const sql::SQLException& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }

        if(resultStr.empty())
        {
            throw Utils::HttpNotFound("Pair not found.");
        }

        return resultStr;
    }
}
