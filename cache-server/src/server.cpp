#include <iostream>
#include <memory>
#include <thread>

#include "server_http.hpp"
#include "client_https.hpp"
#include "popl.hpp"

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;

static const char* executableName = "server";

int main(int argc, char **argv)
{
    try
    {
        popl::OptionParser commandLineParser("Allowed options");

        commandLineParser.add<popl::Switch>("h", "help", "Print this help message");
        commandLineParser.add<popl::Value<int>>("p", "port", "Port to listen on");
        commandLineParser.add<popl::Value<std::string>>("c", "api-config", "Path to the configuration file of the API server");
        commandLineParser.parse(argc, argv);        

        if(commandLineParser.get_option<popl::Switch>("help")->is_set())
        {
            std::cout << commandLineParser.help() << std::endl;
            return 0;
        }

        std::string errorMessage;

        auto portOption = commandLineParser.get_option<popl::Value<int>>("port");
        auto apiConfigOption = commandLineParser.get_option<popl::Value<std::string>>("api-config");

        if(portOption->is_set())
        {
            if(portOption->value() < 0 || portOption->value() > 65535)
            {
                errorMessage += "Invalid port number!\n";
            }
        }
        else
        {
            errorMessage += "Port number not specified!\n";
        }

        if(apiConfigOption->is_set())
        {
            const auto pathToApiConfig = apiConfigOption->value();
            std::ifstream configFile(pathToApiConfig);
            if(!configFile.good())
            {
                errorMessage += "Invalid path to the configuration file!\n";
            }
        }
        else
        {
            errorMessage += "Path to the configuration file not specified!\n";
        }

        if(!errorMessage.empty())
        {
            std::cerr << errorMessage << std::endl;
            return 1;
        }

        HttpServer server;
        server.config.port = portOption->value();
        server.default_resource["GET"] = [](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
        {
            response->write("Hello, World!");
        };
        
        std::thread serverThread([&server]()
        {
            server.start();
        });

        std::cout << "Server started on port " << server.config.port << "..." << std::endl;

        popl::OptionParser apiConfigParser;
        apiConfigParser.add<popl::Value<std::string>>("h", "global.host", "Host of the API server");
        apiConfigParser.add<popl::Value<int>>("p", "global.port", "Port of the API server");
        apiConfigParser.parse(apiConfigOption->value());

        const std::string configServerUri = apiConfigParser.get_option<popl::Value<std::string>>("global.host")->value() + ":"
                                   + std::to_string(apiConfigParser.get_option<popl::Value<int>>("global.port")->value());

        std::cout << "Connecting to API server at " << configServerUri << std::endl;

        HttpsClient client(configServerUri, false);

        while(true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Sending GET request to " << configServerUri << "..." << std::endl;
            SimpleWeb::CaseInsensitiveMultimap header;
            header.emplace("Content-Type", "application/json");
            auto response = client.request("GET", "/config/users", "", header);
            std::cout << "Response received!\n"
                      << "Status Code: " << response->status_code << "\n"
                      << "Content: " << response->content.string() << std::endl;
        }
        
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