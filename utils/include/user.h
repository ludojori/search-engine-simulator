#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace Utils
{
    enum class UserType
    {
        External = 0,
        Internal,
        Manager
    };

    static const std::unordered_map<UserType, std::string> userTypes = {
        { UserType::External,   "external" },
        { UserType::Internal,   "internal" },
        { UserType::Manager,    "manager" }
    };

    inline std::string getUserTypeString(const UserType val)
    {
        return userTypes.find(val)->second;
    }

    struct User
    {
        std::string username;
        std::string password;
        UserType type;

        std::string serialize() const
        {
            try
            {
                boost::property_tree::ptree ptree;
                std::ostringstream buf;

                ptree.put("username", username);
                ptree.put("type", getUserTypeString(type));

                boost::property_tree::write_json(buf, ptree, false);

                return buf.str();
            }
            catch(const std::exception& e)
            {
                std::cerr << "An error occured while serializing a pair: " << e.what() << '\n';
                return std::string();
            }
        }
    };

    User parseUser(const std::string& serializedUser)
    {
        try
        {
            std::istringstream is(serializedUser);
            boost::property_tree::ptree ptree;
            boost::property_tree::read_json(is, ptree);
            User user;

            user.username = ptree.get<std::string>("username");
            user.password = ptree.get<std::string>("password");
            user.type = static_cast<UserType>(ptree.get<int>("type"));

            return user;
        }
        catch(const std::exception& e)
        {
            std::cerr << "An error occured while deserializing a user: " << e.what() << '\n';
            return User();
        }
    }
}