#include <iostream>
#include <memory>

#include "server_https.hpp"
#include "options.h"
#include "provider.h"
#include "user.h"
#include "pair.h"
#include "server-exceptions.h"
#include <filesystem>

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;
using namespace Utils;

static const char* executableName = "server";

/**
 * Apply options to server settings prior to initialization.
 */
void configure(HttpsServer& server, const ApiServer::Options& options)
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
 * Defines server endpoints and behavior.
 */
void addResources(HttpsServer& server, std::shared_ptr<ApiServer::Provider> provider)
{
	server.resource["^/config/users$"]["POST"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			const std::string content = request->content.string();
			const auto user = parseUser(content);
			provider->insertUser(user);
			*response << HTTPCREATED << "\r\n"
					  << "Content-Type: application/json\r\n"
					  << "Content-Length: " << content.length()
					  << "\r\n\r\n";
		}
		catch(const HttpException& e)
		{
			*response << "Content-Length: " << strlen(e.what()) << "\r\n"
					  << e.what()
					  << "\r\n\r\n";
		}
	};

	server.resource["^/config/users$"]["GET"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			const auto responseStr = std::string("{\"users\":") + provider->getUsers() + "}";
			*response << HTTPOK << "\r\n"
					  << "Content-Type: application/json\r\n"
					  << "Content-Length: " << responseStr.size() << "\r\n"
					  << responseStr
					  << "\r\n\r\n";
		}
		catch(const HttpException& e)
		{
			*response << "Content-Length: " << strlen(e.what()) << "\r\n"
					  << e.what()
					  << "\r\n\r\n";
		}
	};

	server.resource["^/config/pairs$"]["POST"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			const std::string content = request->content.string();
			const auto pair = parsePair(content);
			provider->insertPair(pair);
			*response << HTTPCREATED << "\r\n"
					  << "Content-Type: application/json\r\n"
					  << "Content-Length: " << content.length()
					  << "\r\n\r\n";
		}
		catch(const HttpException& e)
		{
			*response << "Content-Length: " << strlen(e.what()) << "\r\n"
					  << e.what()
					  << "\r\n\r\n";
		}
	};

	server.resource["^/config/pairs$"]["GET"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			const std::string serializedPairs = provider->getPairs();
			*response << HTTPOK << "\r\n"
					  << "Content-Type: application/json\r\n"
					  << "Content-Length: " << serializedPairs.size() << "\r\n"
					  << "{\"pairs\":" << serializedPairs << "}"
					  << "\r\n\r\n";
		}
		catch(const HttpException& e)
		{
			*response << "Content-Length: " << strlen(e.what()) << "\r\n"
					  << e.what()
					  << "\r\n\r\n";
		}
	};

	server.resource["^/config/pairs/[A-Z]{3}-[A-Z]{3}$"]["GET"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			const std::string& filter = request->path_match[2];
			const size_t delimiter = filter.find('-');
			const std::string serializedPair = provider->getPair(filter.substr(0, delimiter), filter.substr(delimiter + 1));
			*response << HTTPOK << "\r\n"
					  << "Content-Type: application/json\r\n"
					  << "Content-Length: " << serializedPair.size() << "\r\n"
					  << "{\"pair\":" << serializedPair << "}"
					  << "\r\n\r\n";
		}
		catch(const HttpException& e)
		{
			*response << "Content-Length: " << strlen(e.what()) << "\r\n"
					  << e.what()
					  << "\r\n\r\n";
		}
	};
}

int main(int /*argc*/, char **argv)
{
	try
	{
		const auto execPath = std::string(argv[0]);
		const auto execPathNoFilename = execPath.substr(0, execPath.size() - strlen(executableName));
		const auto configPath = execPath.substr(0, execPath.size() - 6) + "config.ini";
		std::cout << "Parsing " << configPath << "..." << std::endl;

		ApiServer::Options options(configPath);

		std::cout << "Done." << std::endl;
		std::cout << "Server is running on port " << options.getPort() << "..." << std::endl;

		auto provider = std::make_shared<ApiServer::Provider>(options.getMySqlHost(),
									 						  options.getMySqlPort(),
									 						  options.getMySqlUsername(),
									 						  options.getMySqlPassword(),
															  options.getMySqlDatabase());

		HttpsServer server(execPathNoFilename + options.getCertificatePath(),
						   execPathNoFilename + options.getPrivateKeyPath());

		configure(server, options);
		addResources(server, provider);

		server.start();

		// VV: service goal
		/**
		 * DDoS mitigation: residential proxy, WAF, Load Balancer
		 * Sql Injection mitigation: user roles, request schemas, sql driver security
		 */
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
	
	return 0;
}
