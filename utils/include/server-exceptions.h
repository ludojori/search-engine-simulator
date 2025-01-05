#pragma once

#include <stdexcept>

namespace Utils
{
    class HttpException : public std::runtime_error
    {
    public:
        HttpException(const std::string& msg) : std::runtime_error(msg) {}

        virtual int errorCode() const noexcept = 0;
    };

    class HttpBadRequest : public HttpException
    {
    public:
        HttpBadRequest(const std::string& msg) : HttpException(msg) {}

    private:
        int errorCode() const noexcept override { return 400; }
    };

    class HttpUnauthorized : public HttpException
    {
    public:
        HttpUnauthorized(const std::string& msg) : HttpException(msg) {}

    private:
        int errorCode() const noexcept override { return 401; }
    };

    class HttpForbidden : public HttpException
    {
    public:
        HttpForbidden(const std::string& msg) : HttpException(msg) {}

    private:
        int errorCode() const noexcept override { return 403; }
    };

    class HttpNotFound : public HttpException
    {
    public:
        HttpNotFound(const std::string& msg) : HttpException(msg) {}

    private:
        int errorCode() const noexcept override { return 404; }
    };

    class HttpStateConflict : public HttpException
    {
        HttpStateConflict(const std::string& msg) : HttpException(msg) {}

    private:
        int errorCode() const noexcept override { return 409; }
    };

    class HttpInternalServerError : public HttpException
    {
    public:
        HttpInternalServerError(const std::string& msg) : HttpException(msg) {}

    private:
        int errorCode() const noexcept override { return 500; }
    };
}