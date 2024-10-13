#include <iostream>

#include "server_https.hpp"
#include "client_https.hpp"

#include "options.h"

void run()
{
	std::cout << "Initiating server..." << std::endl;
	

	SimpleWeb::Server<SimpleWeb::HTTPS> server("server.crt", "server.key");
	server.config.port = 8080;

	std::thread server_thread([&server]{
		server.start();
	});
	
	SimpleWeb::Client<SimpleWeb::HTTPS> client("localhost:8080", false);
	std::cout << "Server is running..." << std::endl;
}

int main()
{
	run();

	return 0;
}
