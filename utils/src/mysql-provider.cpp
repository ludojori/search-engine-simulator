#include "mysql-provider.h"

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "pointer-wrapper.h"

namespace Utils
{
    MySqlProvider::MySqlProvider(const std::string& dbHost, const int dbPort, const std::string& username, const std::string& password, const std::string& database) :
                        _dbHost(dbHost), _dbPort(dbPort), _username(username), _password(password), _database(database)
    {
        _driver = get_driver_instance();
        _connection = _driver->connect(_dbHost, _username, _password);
        _connection->setSchema(_database);
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
