#include "provider.h"
#include "mysql_connection.h"
#include "mysql_driver.h"
#include <cppconn/statement.h>
#include "pair.h"
#include "server-exceptions.h"

namespace ApiServer
{
    static const std::string CONN_PROTOCOL = "tcp://";
    static const std::string CONN_DATABASE = "search_engine_simulation";

    Provider::Provider(const std::string& dbHost, const int dbPort, const std::string& username, const std::string& password) :
                        _dbHost(dbHost), _dbPort(dbPort), _username(username), _password(password)
    {
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
            std::string connectionString;
            connectionString += CONN_PROTOCOL;
            connectionString += _dbHost;
            connectionString += ":" + _dbPort;
            connectionString += "/" + CONN_DATABASE;

            auto driver = get_driver_instance();
            auto conn = driver->connect(connectionString, _username, _password);       
            auto stmt = conn->createStatement();

            auto result = stmt->executeQuery("SELECT * FROM users");
            std::string resultStr = "[";
            while(result->next())
            {
                if(resultStr.size() > 1)
                {
                    resultStr += ",";
                }

                resultStr += "{";
                resultStr.append("\"username\": \"").append(result->getString("username")).append("\"");
                resultStr.append("\"type\": ").append(std::to_string(result->getInt("type_id")));
                resultStr += "}";

            }
            resultStr += "]";

            delete result;
            delete stmt;
            delete conn;

            return resultStr;
        }
        catch(const std::exception& e)
        {
            throw Utils::HttpInternalServerError(e.what());
        }
    }

    std::string Provider::getPairs()
    {
        return "getPairs()";
    }

    std::string Provider::getPair(const std::string&, const std::string&)
    {
        return "getPair()";
    }
}