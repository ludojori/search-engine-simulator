#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <set>

#include "server-common.h"
#include "flights-provider.h"

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using namespace Utils;

static const char* executableName = "server";

/**
 * Define server endpoints and behavior.
 */
void addResources(HttpServer& server, std::shared_ptr<RealtimeServer::Provider> provider, const std::set<std::string>& blacklistedIPs)
{
    server.default_resource["GET"] = [blacklistedIPs](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
    {
        try
        {
            validateNotBlacklisted(request, blacklistedIPs);
        }
        catch(const HttpException& e)
        {
            response->write(extractErrorCode(e), e.what());
            return;
        }

        try
        {
            response->write("This is the default resource. Try: /flights?origin=origin&destination=destination\n");
        }
        catch(const std::exception& e)
        {
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error, std::string("Unexpected error: ") + e.what());
        }
    };

    server.resource["^/flights$"]["GET"] = [provider, blacklistedIPs](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
    {
        try
        {
            validateNotBlacklisted(request, blacklistedIPs);

            verifyHeaders(request->header);
			auto [username, password] = parseBasicAuthCredentials(request->header);

            if(!provider->isAuthenticated(username, password))
            {
                std::cout << "[DEBUG] Authentication failed for user: " << username << " password: " << password << std::endl;
                throw HttpUnauthorized("Invalid username or password.");
            }

            if(!provider->isAuthorized(username, UserType::External) &&
               !provider->isAuthorized(username, UserType::Internal) &&
               !provider->isAuthorized(username, UserType::Manager) &&
               !provider->isAuthorized(username, UserType::Admin))
            {
                throw HttpForbidden("User " + username + " is not authorized to perform this action.");
            }

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
		const auto configPath = execPath + "config.ini";

        std::cout << "Parsing " << configPath << "..." << std::endl;

        Options options(configPath);

        std::cout << "Done." << std::endl;

        HttpServer server;
        
        auto provider = std::make_shared<RealtimeServer::Provider>(options.getMySqlHost(),
                                                                   options.getMySqlPort(),
                                                                   options.getMySqlUsername(),
                                                                   options.getMySqlPassword(),
                                                                   options.getMySqlDatabase());
        
        configure(server, options);
        addResources(server, provider, options.getBlacklistedIPs());

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