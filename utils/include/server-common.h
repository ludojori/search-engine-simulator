#pragma once

#include <set>

// This removes an annoying compilation message.
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/json/src.hpp>
#include <boost/beast/core/detail/base64.hpp>

#include "server_https.hpp"
#include "server-exceptions.h"
#include "options.h"

namespace Utils
{
    /**
     * A helper function for converting custom exception values to library error codes.
     */
    SimpleWeb::StatusCode extractErrorCode(const HttpException& e)
    {
        using namespace SimpleWeb;

        switch(e.errorCode())
        {
            case 400: 	return StatusCode::client_error_bad_request;
            case 401: 	return StatusCode::client_error_unauthorized;
            case 403: 	return StatusCode::client_error_forbidden;
            case 404: 	return StatusCode::client_error_not_found;
            case 409: 	return StatusCode::client_error_conflict;
            default: 	return StatusCode::server_error_internal_server_error;
        }
    }

    /**
     * Throws in case of an error.
     */
    void verifyHeaders(const SimpleWeb::CaseInsensitiveMultimap& headers)
    {
        auto contentType = headers.find("Content-Type");
        if(contentType == headers.end())
        {
            throw HttpBadRequest("Missing Content-Type header.");
        }

        if(contentType->second != "application/json")
        {
            throw HttpBadRequest("Invalid Content-Type header.");
        }

        auto authHeader = headers.find("Authorization");
        if(authHeader == headers.end())
        {
            throw HttpBadRequest("Missing Authorization header.");
        }
    }

    /**
     * Extracts the value of the Authorization header from the request headers.
     * Only accepts the Basic scheme. Throws HttpBadRequest if missing or invalid.
     */
    std::string extractAuthHeader(const SimpleWeb::CaseInsensitiveMultimap& headers)
    {
        auto authHeader = headers.find("Authorization");
        const std::string& value = authHeader->second;
        const std::string prefix = "Basic ";

        if (value.substr(0, prefix.size()) != prefix)
        {
            throw HttpBadRequest("Authorization header must use Basic scheme.");
        }

        return value.substr(prefix.size());
    }

    std::pair<std::string, std::string> parseBasicAuthCredentials(const SimpleWeb::CaseInsensitiveMultimap& headers)
    {
        std::string encoded = extractAuthHeader(headers);

        // Decode Base64 using boost::beast::detail::base64::decode
        std::string decoded;
        {
            std::vector<unsigned char> buf((encoded.size() * 3) / 4 + 1);
            auto result = boost::beast::detail::base64::decode(buf.data(), encoded.data(), encoded.size());
            decoded.assign(reinterpret_cast<const char*>(buf.data()), result.first);
        }

        // Split at the first colon
        auto pos = decoded.find(':');
        if (pos == std::string::npos)
            throw HttpBadRequest("Malformed Basic auth credentials.");
        std::string username = decoded.substr(0, pos);
        std::string password = decoded.substr(pos + 1);
        return {username, password};
    }

    std::string hashPasswordSHA256(const std::string& password)
    {
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hashLen = 0;
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();

        EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
        EVP_DigestUpdate(ctx, password.data(), password.size());
        EVP_DigestFinal_ex(ctx, hash, &hashLen);
        EVP_MD_CTX_free(ctx);

        std::ostringstream oss;
        for (unsigned int i = 0; i < hashLen; ++i)
        {
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }

        return oss.str();
    }

    /**
     * Apply options to server settings prior to initialization.
     */
    template<typename ServerType>
    void configure(ServerType& server, const Options& options)
    {
        server.config.address = options.getHost();
        server.config.port = options.getPort();
        server.config.reuse_address = false;
        server.config.max_request_streambuf_size = options.getMaxRequestStreambufSize();
        server.config.thread_pool_size = options.getThreadPoolSize();
        server.config.timeout_content = options.getTimeoutContent();
        server.config.timeout_request = options.getTimeoutRequest();
    }

    template<typename RequestType>
    void validateNotBlacklisted(std::shared_ptr<RequestType> request, const std::set<std::string>& blacklistedIPs)
    {
        if (blacklistedIPs.empty())
            return;

        auto remoteEndpoint = request->remote_endpoint_address();
        if (blacklistedIPs.find(remoteEndpoint) != blacklistedIPs.end())
        {
            throw HttpForbidden("IP address " + remoteEndpoint + " is blacklisted.");
        }
    }
}