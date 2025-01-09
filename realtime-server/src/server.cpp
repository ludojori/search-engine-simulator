#include <iostream>
#include <memory>
#include <thread>

#include "server_http.hpp"
#include "client_https.hpp"
#include "options.h"
#include "flights-provider.h"

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;

static const char* executableName = "server";

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
            response->write("Hello, World!");
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    };
}

int main(int argc, char **argv)
{
    try
    {
        const auto execPathWithFilename = std::string(argv[0]);
		const auto execPath = execPathWithFilename.substr(0, execPathWithFilename.size() - strlen(executableName));
		auto configPath = execPathWithFilename.substr(0, execPathWithFilename.size() - 6) + "config.ini";
        auto apiConfigPath = execPath + "../../api-server/bin/config.ini";

        popl::OptionParser commandLineParser("Allowed options");

        commandLineParser.add<popl::Switch>("h", "help", "Print this help message");
        commandLineParser.add<popl::Value<std::string>, popl::Attribute::optional>("c", "config", "Path to the configuration file of this server");
        commandLineParser.add<popl::Value<std::string>, popl::Attribute::optional>("a", "api-config", "Path to the configuration file of the API server");
        commandLineParser.parse(argc, argv);        

        if(commandLineParser.get_option<popl::Switch>("help")->is_set())
        {
            std::cout << commandLineParser.help() << std::endl;
            return 0;
        }

        std::string errorMessage;

        auto configOption = commandLineParser.get_option<popl::Value<std::string>>("config");
        auto apiConfigOption = commandLineParser.get_option<popl::Value<std::string>>("api-config");

        if(configOption->is_set())
        {
            configPath = configOption->value();
        }
        else
        {
            std::cout << "Path to the configuration file not specified, defaulting to realtime-server/bin/config.ini" << std::endl;
        }

        {
            std::ifstream configFile(configPath);

            if(!configFile.good())
            {
                errorMessage += "Configuration file not found!\n";
            }
        }

        if(apiConfigOption->is_set())
        {
            apiConfigPath = apiConfigOption->value();
        }
        else
        {
            std::cout << "Path to the API server configuration file not specified, defaulting to api-server/bin/config.ini" << std::endl;
        }

        {
            std::ifstream configFile(apiConfigPath);

            if(!configFile.good())
            {
                errorMessage += "API server configuration file not found!\n";
            }
        }

        if(!errorMessage.empty())
        {
            std::cerr << "Fatal error: " << errorMessage << std::endl;
            return 1;
        }

        Utils::Options configOptions(configPath);
        HttpServer server;
        server.config.port = configOptions.getPort();

        configure(server, configOptions);

        auto provider = std::make_shared<RealtimeServer::FlightsProvider>(configOptions.getMySqlHost(),
                                                                          configOptions.getMySqlPort(),
                                                                          configOptions.getMySqlUsername(),
                                                                          configOptions.getMySqlPassword(),
                                                                          configOptions.getMySqlDatabase());

        addResources(server, provider);
        
        std::thread serverThread([&server]()
        {
            server.start();
        });

        std::cout << "Server started on port " << server.config.port << "..." << std::endl;

        Utils::Options apiConfigOptions(apiConfigPath);

        const std::string configServerUri = apiConfigOptions.getHost() + ":" + std::to_string(apiConfigOptions.getPort());

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