#pragma once

#include <string>

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

    enum class PermissionLevel
    {
        None = 0,
        Read,
        Write
    };

    enum class OperationType
    {
        Read = 0,
        Update, // Includes both insert and update
        Delete
    };

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

        /**
         * @brief For SELECT queries. Afterwards call executeQuery() on the returned object.
         */
        PointerWrapper<sql::Statement> createStatement();

        /**
         * @brief For INSERT, UPDATE, DELETE queries.
         */
        PointerWrapper<sql::PreparedStatement> prepareStatement(const std::string& stmtStr);

        virtual ~MySqlProvider();

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