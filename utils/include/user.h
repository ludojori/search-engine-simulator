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

        std::string serialize()
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
}