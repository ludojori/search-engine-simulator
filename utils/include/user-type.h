#pragma once

#include <string>
#include <unordered_map>

namespace Utils
{
    enum class UserType : int
    {
        External = 1,
        Internal = 2,
        Manager = 3,
        Admin = 4
    };

    static const std::unordered_map<UserType, std::string> userTypes = {
        { UserType::External,   "external" },
        { UserType::Internal,   "internal" },
        { UserType::Manager,    "manager" },
        { UserType::Admin,      "admin" }
    };

    inline std::string getUserTypeString(const UserType val)
    {
        auto it = userTypes.find(val);
        return it != userTypes.end() ? it->second : "unknown";
    }
}