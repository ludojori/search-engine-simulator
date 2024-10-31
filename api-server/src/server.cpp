#include <iostream>
#include <memory>
#include <mysql_driver.h>

#include "server_https.hpp"
#include "options.h"
#include "provider.h"
#include "server-exceptions.h"

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;
using namespace Utils;

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
			provider->insertUser(content);
			*response << HTTPCREATED << "\r\n"
					  << "Content-Length: " << content.length() << "\r\n\r\n";
		}
		catch(const HttpException& e)
		{
			*response << e.what();
		}
	};

	server.resource["^/config/users$"]["GET"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			const std::string users = provider->getUsers();
			*response << HTTPOK << "\r\n"
					  << "{users:" << users << "}";
		}
		catch(const HttpException& e)
		{
			*response << e.what();
		}
	};

	server.resource["^/config/pairs$"]["POST"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			const std::string content = request->content.string();
			provider->insertPair(content);
			*response << HTTPCREATED << "\r\n"
					  << "Content-Length: " << content.length() << "\r\n\r\n";
		}
		catch(const HttpException& e)
		{
			*response << e.what();
		}
	};

	server.resource["^/config/pairs$"]["GET"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			const std::string pairs = provider->getPairs();
			*response << HTTPOK << "\r\n"
					  << "{pairs:" << pairs << "}";
		}
		catch(const HttpException& e)
		{
			*response << e.what();
		}
	};

	server.resource["^/config/pairs/[A-Z]{3}-[A-Z]{3}$"]["GET"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			const std::string filter = request->path_match[2];
			const size_t delimiter = filter.find('-');
			const std::string pair = provider->getPair(pair.substr(0, delimiter), pair.substr(delimiter + 1));
			*response << HTTPOK << "\r\n"
					  << "{pair:" << pair << "}";
		}
		catch(const HttpException& e)
		{
			*response << e.what();
		}
	};
}

int main()
{
	try
	{
		std::cout << "Parsing config.ini..." << std::endl;

		ApiServer::Options options("config.ini");

		std::cout << "Done." << std::endl;
		std::cout << "Initiating server on port " << options.getPort() << "..." << std::endl;

		auto provider = std::make_shared<ApiServer::Provider>(options.getMySqlHost(),
									 						  options.getMySqlPort(),
									 						  options.getMySqlUsername(),
									 						  options.getMySqlPassword());

		HttpsServer server(options.getCertificatePath(), options.getPrivateKeyPath());

		configure(server, options);
		addResources(server, provider);

		server.start();
	}
	catch(const popl::invalid_option& e)
	{
		std::cerr << "Failed to parse options, error: " << e.what() << '\n';
	}
	catch(const std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what() << '\n';
	}
	
	return 0;
}
