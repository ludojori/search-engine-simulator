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

    enum class PermissionLevel : uint8_t
    {
        None  = 0,
        Read  = 1 << 0,
        Write = 1 << 1,
        Admin = 1 << 2 // Optional, for future extensibility
    };

    inline PermissionLevel operator|(PermissionLevel a, PermissionLevel b)
    {
        return static_cast<PermissionLevel>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    inline PermissionLevel operator&(PermissionLevel a, PermissionLevel b)
    {
        return static_cast<PermissionLevel>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    inline bool hasPermission(PermissionLevel userPerms, PermissionLevel required)
    {
        return (static_cast<uint8_t>(userPerms) & static_cast<uint8_t>(required)) == static_cast<uint8_t>(required);
    }

    inline const char* toString(PermissionLevel level)
    {
        switch(level)
        {
            case PermissionLevel::None: return "None";
            case PermissionLevel::Read: return "Read";
            case PermissionLevel::Write: return "Write";
            case PermissionLevel::Admin: return "Admin";
            default: return "Unknown";
        }
    }

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

        bool isAuthenticated(const std::string& username,
                             const std::string& password);

        bool isAuthorized(const std::string& username,
                          OperationType operationType, 
                          PermissionLevel permissionLevel);

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