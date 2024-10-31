#pragma once

#include <stdexcept>

namespace Utils
{
    static const std::string HTTPOK = "HTTP/1.1 200 OK";
    static const std::string HTTPCREATED = "HTTP/1.1 201 Created";

    class HttpException : public std::exception
    {
    public:
        HttpException(const std::string& msg) : _msg(msg) {}
        virtual const char* what() const noexcept override
        {
            const int msgLength = _msg.length();
            const auto prefix = std::string("HTTP/1.1 ")
                                .append(errorCode())
                                .append("\r\nContent-Length: ")
                                .append(std::to_string(msgLength))
                                .append("\r\nContent-Type: application/json")
                                .append("\r\n\r\n{ \"message\": \"");
            _msg.insert(0, prefix);
            _msg.append("\" }\r\n");

            return _msg.c_str();
        }

    private:
        virtual const char* errorCode() const noexcept = 0;
        mutable std::string _msg;
    };

    class HttpBadRequest : public HttpException
    {
    public:
        HttpBadRequest(const std::string& msg) : HttpException(msg) {}

    private:
        const char* errorCode() const noexcept override { return "400 Bad Request"; }
    };

    class HttpUnauthorized : public HttpException
    {
    public:
        HttpUnauthorized(const std::string& msg) : HttpException(msg) {}

    private:
        const char* errorCode() const noexcept override { return "401 Unauthorized"; }
    };

    class HttpForbidden : public HttpException
    {
    public:
        HttpForbidden(const std::string& msg) : HttpException(msg) {}

    private:
        const char* errorCode() const noexcept override { return "403 Forbidden"; }
    };

    class HttpNotFound : public HttpException
    {
    public:
        HttpNotFound(const std::string& msg) : HttpException(msg) {}

    private:
        const char* errorCode() const noexcept override { return "404 Not Found"; }
    };

    class HttpStateConflict : public HttpException
    {
        HttpStateConflict(const std::string& msg) : HttpException(msg) {}

    private:
        const char* errorCode() const noexcept override { return "409 State Conflict"; }
    };

    class HttpInternalServerError : public HttpException
    {
    public:
        HttpInternalServerError(const std::string& msg) : HttpException(msg) {}

    private:
        const char* errorCode() const noexcept override { return "500 Internal Server Error"; }
    };
}