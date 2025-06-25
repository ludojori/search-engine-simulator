#pragma once

#include "mysql-provider.h"

namespace Utils
{
    class User;
    class Pair;
}

namespace ConfigServer
{
    class Provider final : public Utils::MySqlProvider
    {
    public:
        explicit Provider(const std::string& dbHost,
                          const int dbPort,
                          const std::string& username,
                          const std::string& password,
                          const std::string& database);

        /**
         * @brief Uses a prepared statement.
         */
        void insertUserSafe(const Utils::User& user);

        /**
         * @brief Uses string concatenation to build a query which can lead to SQL injection vulnerabilities.
         */
        void insertUserUnsafe(const Utils::User& user);

        /**
         * @brief Uses a prepared statement.
         */
        void insertPairSafe(const Utils::Pair& pair);

        /**
         * @brief Lists all users in the database. Such methods should only be used for debugging purposes by
         * internal users like developers, or by other explicitly authorized users.
         */
        std::string getUsers();

        /**
         * @brief Lists all pairs in the database. Such methods should only be used for debugging purposes by
         * internal users like developers, or by other explicitly authorized users.
         */
        std::string getPairs();

        /**
         * @brief A not-so-unsafe method which can only return a single result, but still relies
         * on input sanitization.
         */
        std::string getPair(const std::string& origin, const std::string& destination);

        /**
         * @brief Uses a while-loop which enables returning multiple pairs that match the criteria.
         * If the input is not sanitized, it may lead to SQL injection vulnerabilities.
         */
        std::string getPairUnsafe(const std::string& origin, const std::string& destination);
    };
}