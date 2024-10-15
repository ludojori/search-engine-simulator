#include <iostream>

#include "server_https.hpp"
#include "client_https.hpp"

#include "options.h"


int main()
{
	try
	{
		std::cout << "Parsing config.ini..." << std::endl;

		ApiServer::Options options("config.ini");

		std::cout << "Done." << std::endl;
		std::cout << "Initiating server..." << std::endl;

		SimpleWeb::Server<SimpleWeb::HTTPS> server(options.getCertificatePath(), options.getPrivateKeyPath());
		server.config.address = options.getHost();
		server.config.port = options.getPort();
		server.config.reuse_address = false;
		server.config.max_request_streambuf_size = options.getMaxRequestStreambufSize();
		server.config.thread_pool_size = options.getThreadPoolSize();
		server.config.timeout_content = options.getTimeoutContent();
		server.config.timeout_request = options.getTimeoutRequest();

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
