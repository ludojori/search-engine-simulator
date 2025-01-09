#include <iostream>
#include <memory>

#include "server_https.hpp"
#include "options.h"
#include "config-provider.h"
#include "user.h"
#include "pair.h"
#include "server-exceptions.h"

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;
using namespace Utils;

static const char* executableName = "server";

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
		throw HttpBadRequest("Missing Content-Type header. Expected 'application/json'.");
	}
}

/**
 * Apply options to server settings prior to initialization.
 */
void configure(HttpsServer& server, const Utils::Options& options)
{
	server.config.address = options.getHost();
	server.config.port = options.getPort();
	server.config.reuse_address = false;
	server.config.max_request_streambuf_size = options.getMaxRequestStreambufSize();
	server.config.thread_pool_size = options.getThreadPoolSize();
	server.config.timeout_content = options.getTimeoutContent();
	server.config.timeout_request = options.getTimeoutRequest();
}

/**
 * Define server endpoints and behavior.
 */
void addResources(HttpsServer& server, std::shared_ptr<ApiServer::ConfigProvider> provider)
{
	server.default_resource["GET"] = [](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			verifyHeaders(request->header);
			response->write("This is the default resource.");
		}
		catch(const std::exception& e)
		{
			response->write(SimpleWeb::StatusCode::server_error_internal_server_error, std::string("The server encountered an error."));
		}
	};

	server.resource["^/config/users$"]["POST"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			verifyHeaders(request->header);

			const std::string content = request->content.string();
			const Utils::User user = parseUser(content);
			
			provider->insertUser(user);
			response->write(SimpleWeb::StatusCode::success_created, content);
		}
		catch(const HttpException& e)
		{
			response->write(extractErrorCode(e), e.what());
		}
	};

	server.resource["^/config/users$"]["GET"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			verifyHeaders(request->header);
			response->write("{\"users\":" + provider->getUsers() + "}");
		}
		catch(const HttpException& e)
		{
			response->write(extractErrorCode(e), e.what());
		}
	};

	server.resource["^/config/pairs$"]["POST"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			verifyHeaders(request->header);

			const std::string content = request->content.string();
			const Utils::Pair pair = parsePair(content);

			provider->insertPair(pair);
			response->write(SimpleWeb::StatusCode::success_created, content);
		}
		catch(const HttpException& e)
		{
			response->write(extractErrorCode(e), e.what());
		}
	};

	server.resource["^/config/pairs$"]["GET"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			verifyHeaders(request->header);
			response->write(provider->getPairs());
		}
		catch(const HttpException& e)
		{
			response->write(extractErrorCode(e), e.what());
		}
	};

	server.resource["^/config/pairs/[A-Z]{3}-[A-Z]{3}$"]["GET"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			verifyHeaders(request->header);

			const std::string& filter = request->path_match[2];
			const size_t delimiter = filter.find('-');

			response->write(provider->getPair(filter.substr(0, delimiter), filter.substr(delimiter + 1)));
		}
		catch(const HttpException& e)
		{
			response->write(extractErrorCode(e), e.what());
		}
	};
}

int main(int /*argc*/, char **argv)
{
	try
	{
		const auto execPathWithFilename = std::string(argv[0]);
		const auto execPath = execPathWithFilename.substr(0, execPathWithFilename.size() - strlen(executableName));
		const auto configPath = execPathWithFilename.substr(0, execPathWithFilename.size() - 6) + "config.ini";

		std::cout << "Parsing " << configPath << "..." << std::endl;

		Utils::Options options(configPath);

		std::cout << "Done." << std::endl;
		std::cout << "Server is running on port " << options.getPort() << "..." << std::endl;

		auto provider = std::make_shared<ApiServer::ConfigProvider>(options.getMySqlHost(),
									 						  		options.getMySqlPort(),
									 						  		options.getMySqlUsername(),
									 						  		options.getMySqlPassword(),
															  		options.getMySqlDatabase());

		HttpsServer server(execPath + options.getCertificatePath(), execPath + options.getPrivateKeyPath());

		configure(server, options);
		addResources(server, provider);

		server.start();

		// VV: service goal
		/**
		 * DDoS mitigation: residential proxy, WAF, Load Balancer
		 * Sql Injection mitigation: user roles, request schemas, sql driver security
		 */

		return 0;
	}
	catch(const popl::invalid_option& e)
	{
		std::cerr << "Failed to parse options, error: " << e.what() << '\n';
		return 1;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what() << '\n';
		return 2;
	}
}
