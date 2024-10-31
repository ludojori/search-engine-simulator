#include "provider.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include "pair.h"

namespace ApiServer
{
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
        return "getUsers()";
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