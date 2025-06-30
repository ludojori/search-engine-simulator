#pragma once

#include <set>

#include "popl.hpp"

namespace Utils
{
    class Options
    {
    public:
        explicit Options(const std::string& pathToConfig);
        Options(const Options&) = delete;
        Options& operator=(const Options&) = delete;

        std::string getHost() const;
        int getPort() const;
        unsigned int getMaxRequestStreambufSize() const;
        unsigned int getThreadPoolSize() const;
        unsigned int getTimeoutContent() const;
        unsigned int getTimeoutRequest() const;

        std::string getCertificatePath() const;
        std::string getPrivateKeyPath() const;
        std::set<std::string> getBlacklistedIPs() const;

        std::string getMySqlHost() const;
        int getMySqlPort() const;
        std::string getMySqlUsername() const;
        std::string getMySqlPassword() const;
        std::string getMySqlDatabase() const;

    private:
        popl::OptionParser _op;

    };
}