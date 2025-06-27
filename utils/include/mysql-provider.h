#pragma once

#include "user-type.h"

namespace sql
{
    class Driver;
    class Connection;
    class Statement;
    class PreparedStatement;
    class ResultSet;
}

namespace Utils
{
    template<typename RawPtr>
    class PointerWrapper;

    class MySqlProvider
    {
    public:
        explicit MySqlProvider(const std::string& dbHost,
                               const int dbPort,
                               const std::string& username,
                               const std::string& password,
                               const std::string& database);

        MySqlProvider(const MySqlProvider&) = delete;
        MySqlProvider& operator=(const MySqlProvider&) = delete;

        bool isAuthenticated(const std::string& username,
                             const std::string& password);

        bool isAuthorized(const std::string& username,
                          UserType userType);

        virtual ~MySqlProvider();

    protected:
        /**
         * @brief For SELECT queries. Afterwards call executeQuery() on the returned object.
         */
        PointerWrapper<sql::Statement> createStatement();

        /**
         * @brief For INSERT, UPDATE, DELETE queries.
         */
        PointerWrapper<sql::PreparedStatement> prepareStatement(const std::string& stmtStr);

    private:
        const std::string _dbHost;
        const int _dbPort;
        const std::string _username;
        const std::string _password;
        const std::string _database;
        sql::Driver* _driver;
        sql::Connection* _connection;
    };
}