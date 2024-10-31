#include "options.h"
#include <limits>

namespace ApiServer
{
    Options::Options(const std::string& pathToConfig)
    {
        _op.add<popl::Value<std::string>, popl::Attribute::required>("", "global.host", "The server host IP address", "127.0.0.1");
        _op.add<popl::Value<int>, popl::Attribute::required>("p", "global.port", "The server port number", 8080);
        _op.add<popl::Value<unsigned int>, popl::Attribute::optional>("", "global.max_request_streambuf_size", "", 0);
        _op.add<popl::Value<unsigned int>, popl::Attribute::optional>("", "global.thread_pool_size", "", 1);
        _op.add<popl::Value<unsigned int>, popl::Attribute::optional>("", "global.timeout_content", "", 0);
        _op.add<popl::Value<unsigned int>, popl::Attribute::optional>("", "global.timeout_request", "", 0);

        _op.add<popl::Value<std::string>, popl::Attribute::required>("", "security.certificate_path", "", "server.crt");
        _op.add<popl::Value<std::string>, popl::Attribute::required>("", "security.private_key_path", "", "server.key");

        _op.add<popl::Value<std::string>, popl::Attribute::required>("", "mysql.host", "The host used to connect to the MySQL database.", "127.0.0.1");
        _op.add<popl::Value<int>, popl::Attribute::required>("", "mysql.port", "The port used to connect to the MySQL database.", 3306);
        _op.add<popl::Value<std::string>, popl::Attribute::required>("", "mysql.username", "The username to authenticate when connecting to the MySQL database.", "");
        _op.add<popl::Value<std::string>, popl::Attribute::required>("", "mysql.password", "The password for the MySQL user.", "");
        _op.add<popl::Value<std::string>, popl::Attribute::required>("", "mysql.database", "", "");

        _op.parse(pathToConfig);
    }

    std::string Options::getHost() const
    {
        return _op.get_option<popl::Value<std::string>>("global.host")->value_or("127.0.0.1");
    }

    int Options::getPort() const
    {
        return _op.get_option<popl::Value<int>>("global.port")->value_or(443);
    }

    unsigned int Options::getMaxRequestStreambufSize() const
    {
        return _op.get_option<popl::Value<unsigned int>>("global.max_request_streambuf_size")->value_or(std::numeric_limits<unsigned int>::max());
    }

    unsigned int Options::getThreadPoolSize() const
    {
        return _op.get_option<popl::Value<unsigned int>>("global.thread_pool_size")->value_or(1);
    }

    unsigned int Options::getTimeoutContent() const
    {
        return _op.get_option<popl::Value<unsigned int>>("global.timeout_content")->value_or(300);
    }

    unsigned int Options::getTimeoutRequest() const
    {
        return _op.get_option<popl::Value<unsigned int>>("global.timeout_request")->value_or(5);
    }

    std::string Options::getCertificatePath() const
    {
        return _op.get_option<popl::Value<std::string>>("security.certificate_path")->value();
    }

    std::string Options::getPrivateKeyPath() const
    {
        return _op.get_option<popl::Value<std::string>>("security.private_key_path")->value();
    }

    std::string Options::getMySqlHost() const
    {
        return _op.get_option<popl::Value<std::string>>("mysql.host")->value();
    }

    int Options::getMySqlPort() const
    {
        return _op.get_option<popl::Value<int>>("mysql.port")->value();
    }

    std::string Options::getMySqlUsername() const
    {
        return _op.get_option<popl::Value<std::string>>("mysql.username")->value();
    }

    std::string Options::getMySqlPassword() const
    {
        return _op.get_option<popl::Value<std::string>>("mysql.password")->value();
    }

    std::string Options::getMySqlDatabase() const
    {
        return _op.get_option<popl::Value<std::string>>("mysql.database")->value();
    }
}