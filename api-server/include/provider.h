#pragma once

#include <string>
#include <memory>

namespace sql
{
    class Driver;
    class Connection;
}

namespace ApiServer
{
    class Provider
    {
    public:
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

        explicit Provider(const std::string& dbHost,
                          const int dbPort,
                          const std::string& username,
                          const std::string& password,
                          const std::string& database);

        void insertUser(const std::string& content);
        void insertPair(const std::string& content);
        std::string getUsers();
        std::string getPairs();
        std::string getPair(const std::string& origin, const std::string& destination);

        ~Provider();

    private:
        void authorizeOperation(OperationType opType);
        PermissionLevel acquirePermissionLevel();

        const std::string _dbHost;
        const int _dbPort;
        const std::string _username;
        const std::string _password;
        const std::string _database;
        sql::Driver* _driver;
        sql::Connection* _connection;
    };
}