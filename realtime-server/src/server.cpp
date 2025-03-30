#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "server_http.hpp"
#include "client_https.hpp"
#include "options.h"
#include "flights-provider.h"
#include "server-exceptions.h"

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;

static const char* executableName = "server";

/**
 * A helper function for converting custom exception values to library error codes.
 */
SimpleWeb::StatusCode extractErrorCode(const Utils::HttpException& e)
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
 * Apply options to server settings prior to initialization.
 */
void configure(HttpServer& server, const Utils::Options& options)
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
void addResources(HttpServer& server, std::shared_ptr<RealtimeServer::FlightsProvider> provider)
{
    server.default_resource["GET"] = [](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
    {
        try
        {
            response->write("This is the default resource. Try: /flights?origin=origin&destination=destination\n");
        }
        catch(const std::exception& e)
        {
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error, std::string("Unexpected error: ") + e.what());
        }
    };

    server.resource["^/flights$"]["GET"] = [provider](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
    {
        try
        {
            const auto queriesMap = request->parse_query_string();

            std::string origin = "";
            std::string destination = "";

            const auto originIt = queriesMap.find("origin");
            if(originIt != queriesMap.end())
            {
                origin = originIt->second;
            }

            const auto destinationIt = queriesMap.find("destination");
            if(destinationIt != queriesMap.end())
            {
                destination = destinationIt->second;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate complex flight construction.

            response->write(provider->getFlights(origin, destination));
        }
        catch(const Utils::HttpException& e)
        {
            response->write(extractErrorCode(e), e.what());
        }
    };
}

int main(int argc, char **argv)
{
    try
    {
        const auto execPathWithFilename = std::string(argv[0]);
		const auto execPath = execPathWithFilename.substr(0, execPathWithFilename.size() - strlen(executableName));
		const auto configPath = execPath + "config.ini";

        std::cout << "Parsing " << configPath << "..." << std::endl;

        Utils::Options options(configPath);

        std::cout << "Done." << std::endl;

        HttpServer server;
        
        auto provider = std::make_shared<RealtimeServer::FlightsProvider>(options.getMySqlHost(),
                                                                          options.getMySqlPort(),
                                                                          options.getMySqlUsername(),
                                                                          options.getMySqlPassword(),
                                                                          options.getMySqlDatabase());
        
        configure(server, options);
        addResources(server, provider);
        
        std::thread serverThread([&server]()
        {
            server.start();
        });

        std::cout << "Server started on port " << server.config.port << "..." << std::endl;
        
        serverThread.join();

        return 0;
    }
    catch(const popl::invalid_option& e)
    {
        std::cerr << "Error parsing command line arguments: " << e.what() << '\n';
        return 1;
    }
    catch(const std::exception& e)
    {
		std::cerr << "Fatal error: " << e.what() << '\n';
		return 2;
    }
}