#include "mysql-provider.h"

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "pointer-wrapper.h"
#include "server-exceptions.h"

namespace Utils
{
    MySqlProvider::MySqlProvider(const std::string& dbHost, const int dbPort, const std::string& username, const std::string& password, const std::string& database) :
                        _dbHost(dbHost), _dbPort(dbPort), _username(username), _password(password), _database(database)
    {
        _driver = get_driver_instance();
        _connection = _driver->connect(_dbHost, _username, _password);
        _connection->setSchema(_database);
    }

    bool MySqlProvider::isAuthenticated(const std::string& username,
                                        const std::string& password)
    {
        const std::string queryStr = "SELECT COUNT(*) AS user_count FROM users WHERE name=? AND password=?";
        try
        {
            auto stmt = prepareStatement(queryStr);
            stmt->setString(1, username);
            stmt->setString(2, password);
            auto result = PointerWrapper<sql::ResultSet>(stmt->executeQuery());

            if (result->next())
            {
                return result->getInt("user_count") != 0;
            }

            return false;
        }
        catch(const std::exception& e)
        {
            throw HttpInternalServerError(e.what());
        }
    }

    bool MySqlProvider::isAuthorized(const std::string& username,
                                     UserType userType)
    {
        const std::string queryStr = "SELECT COUNT(*) AS user_count FROM users WHERE name=? AND type_id=?";
        try
        {
            auto stmt = prepareStatement(queryStr);
            stmt->setString(1, username);
            stmt->setInt(2, static_cast<int>(userType));
            auto result = PointerWrapper<sql::ResultSet>(stmt->executeQuery());

            if (result->next())
            {
                return result->getInt("user_count") != 0;
            }

            return false;
        }
        catch(const std::exception& e)
        {
            throw HttpInternalServerError(e.what());
        }
    }

    PointerWrapper<sql::Statement> MySqlProvider::createStatement()
    {
        return PointerWrapper(_connection->createStatement());
    }

    PointerWrapper<sql::PreparedStatement> MySqlProvider::prepareStatement(const std::string& stmtStr)
    {
        return PointerWrapper(_connection->prepareStatement(stmtStr));
    }

    MySqlProvider::~MySqlProvider()
    {
        if(_connection)
        {
            delete _connection;
        }

        // Do NOT delete _driver* as required by library manual!
    }
}
