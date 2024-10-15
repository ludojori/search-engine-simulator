#pragma once

#include "popl.hpp"

namespace ApiServer
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

        std::string getMySqlHost() const;
        int getMySqlPort() const;
        std::string getMySqlUsername() const;
        std::string getMySqlPassword() const;
        std::string getMySqlDatabase() const;

    private:
        popl::OptionParser _op;

    };
}