#pragma once

#include <string>

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
                          const std::string& password);

        void insertUser(const std::string& content);
        void insertPair(const std::string& content);
        std::string getUsers();
        std::string getPairs();
        std::string getPair(const std::string& origin, const std::string& destination);

    private:
        void authorizeOperation(OperationType opType);
        PermissionLevel acquirePermissionLevel();

        // mysqlpp connection
        const std::string _dbHost;
        const int _dbPort;
        const std::string _username;
        const std::string _password;
    };
}