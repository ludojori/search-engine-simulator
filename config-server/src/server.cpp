#include <iostream>
#include <memory>
#include <sstream>
#include <iomanip>

// This removes an annoying compilation message.
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/json/src.hpp>
#include <boost/beast/core/detail/base64.hpp>

#include <valijson/adapters/boost_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>
#include <valijson/validation_results.hpp>

#include "server_https.hpp"
#include "options.h"
#include "config-provider.h"
#include "user.h"
#include "pair.h"
#include "server-exceptions.h"

#include <openssl/evp.h>

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;
using namespace Utils;

static const char* executableName = "server";

std::string decodeHexSymbols(const std::string &input)
{
    std::ostringstream decoded;
    const size_t length = input.length();

    for (size_t i = 0; i < length; i++)
	{
        if (input[i] == '%' && i + 2 < length)
		{
            // Extract next two characters as hex digits
            std::string hexValue = input.substr(i + 1, 2);
            int charCode;
            
            // Convert hex string to integer
            std::istringstream(hexValue) >> std::hex >> charCode;
            
            // Append the decoded character
            decoded << static_cast<char>(charCode);
            
            // Skip the processed hex digits
            i += 2;  
        }
		else
		{
            // Append normal characters as is
            decoded << input[i];
        }
    }

    return decoded.str();
}

// Function to read a file into a string
std::string loadFileToString(const std::string& filePath)
{
    std::ifstream fileStream(filePath);
    if (!fileStream.is_open())
	{
        throw std::runtime_error("Could not open file: " + filePath);
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();

    return buffer.str();
}

void validateJson(const std::string& schemaJson, const std::string& targetJson)
{
	boost::system::error_code ec;
	auto schemaDoc = boost::json::parse(schemaJson, ec);
	if (ec)
	{
		throw HttpInternalServerError("Error parsing JSON schema: " + ec.message());
	}

	auto obj = schemaDoc.as_object();
	auto iter = obj.find("$schema");
	if (iter == obj.cend())
	{
		throw HttpInternalServerError("Error finding key $schema");
	}

	iter = obj.find("$ref");
	if (iter != obj.cend())
	{
		throw HttpInternalServerError("Invalid iterator for non-existent key $ref");
	}

	valijson::Schema schema;
    valijson::SchemaParser schemaParser;

	valijson::adapters::BoostJsonAdapter schemaAdapter(schemaDoc);
    std::cout << "Populating schema..." << std::endl;
    schemaParser.populateSchema(schemaAdapter, schema);

	auto targetDoc = boost::json::parse(targetJson, ec);
	if (ec)
	{
		throw HttpInternalServerError("Error parsing JSON target: " + ec.message());
	}

	valijson::Validator validator;
	valijson::ValidationResults results;
	valijson::adapters::BoostJsonAdapter targetAdapter(targetDoc);
	if (validator.validate(schema, targetAdapter, &results))
	{
		std::cout << "Validation succeeded." << std::endl;
		return;
	}

	std::stringstream errorStream;
	errorStream << "Validation failed with the following errors:\n";

	valijson::ValidationResults::Error error;
	unsigned int errorNum = 0;
	while (results.popError(error))
	{
		errorStream << "#" << errorNum << "\n";
		errorStream << "  ";
		for (const std::string& contextElement : error.context)
		{
			errorStream << contextElement << " ";
		}
		errorStream << std::endl;
		errorStream << "    - " << error.description << "\n";
		++errorNum;
	}

	throw HttpBadRequest(errorStream.str());
}

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
void addResources(HttpsServer& server, std::shared_ptr<ApiServer::ConfigProvider> provider, const std::string& pairSchemaJson, const std::string& userSchemaJson)
{
	server.default_resource["GET"] = [](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			const char* helpMessage = "This is the default resource.\n"
									   "The following endpoints are available:\n"
									   "[POST] /config/users/safe\n"
									   "[POST] /config/users/unsafe\n"
									   "[GET]  /config/users\n"
									   "[GET]  /config/pairs/unsafe/{origin}-{destination}\n"
									   "[POST] /config/pairs/safe\n"
									   "[GET]  /config/pairs/safe/{origin}-{destination}\n"
									   ;
			SimpleWeb::CaseInsensitiveMultimap headers = { { "Content-Type", "text/plain" } };
			response->write(helpMessage, headers);
		}
		catch(const std::exception& e)
		{
			response->write(SimpleWeb::StatusCode::server_error_internal_server_error, std::string("Unexpected error: ") + e.what());
		}
	};

	server.resource["^/config/users/safe$"]["POST"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			verifyHeaders(request->header);
			auto [username, password] = parseBasicAuthCredentials(request->header);
			
			const std::string content = request->content.string();
			const Utils::User user = parseUser(content);
			
			provider->insertUserSafe(user);
			response->write(SimpleWeb::StatusCode::success_created, content);
		}
		catch(const HttpException& e)
		{
			response->write(extractErrorCode(e), e.what());
		}
	};

	server.resource["^/config/users/unsafe"]["POST"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		/**
		 * This endpoint allows for SQL injection vulnerabilities in the request body,
		 * by using string concatenation underneath to build a SQL query.
		 * The following curl command demonstrates how to exploit this vulnerability:
		 * curl -k -X POST https://localhost:8080/config/users/unsafe \
  		 * -H "Content-Type: application/json" \
		 * -H "Authorization: Basic YWRtaW46cGFzc3dvcmQ=" \
		 * -d '{"username": "attacker\'); UPDATE users SET type_id=7 WHERE username='\''admin'\''; -- ", "password": "irrelevant", "type": 0}'
		 */
		try
		{
			verifyHeaders(request->header);
			auto [username, password] = parseBasicAuthCredentials(request->header);
			
			const std::string content = request->content.string();
			const Utils::User user = parseUser(content);
			
			provider->insertUserUnsafe(user);
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
			auto [username, password] = parseBasicAuthCredentials(request->header);
			
			response->write("{\"users\":" + provider->getUsers() + "}");
		}
		catch(const HttpException& e)
		{
			response->write(extractErrorCode(e), e.what());
		}
	};

	server.resource["^/config/pairs/safe$"]["POST"] = [provider, pairSchemaJson](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			verifyHeaders(request->header);
			auto [username, password] = parseBasicAuthCredentials(request->header);
			
			const std::string content = request->content.string();
			validateJson(pairSchemaJson, content);
			const Utils::Pair pair = parsePair(content);

			provider->insertPairSafe(pair);
			response->write(SimpleWeb::StatusCode::success_created, content);
		}
		catch(const HttpException& e)
		{
			response->write(extractErrorCode(e), e.what());
		}
	};

	server.resource["^/config/pairs/safe$"]["GET"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			verifyHeaders(request->header);
			auto [username, password] = parseBasicAuthCredentials(request->header);
			
			response->write(provider->getPairs());
		}
		catch(const HttpException& e)
		{
			response->write(extractErrorCode(e), e.what());
		}
	};

	server.resource["^/config/pairs/safe/[A-Z]{3}-[A-Z]{3}$"]["GET"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		try
		{
			verifyHeaders(request->header);
			auto [username, password] = parseBasicAuthCredentials(request->header);
			
			const std::string& path = request->path_match[0];
			const std::string prefix = "/config/pairs/safe/";
			const std::string filter = path.substr(prefix.size());
			const size_t delimiter = filter.find('-');

			response->write(provider->getPair(filter.substr(0, delimiter), filter.substr(delimiter + 1)));
		}
		catch(const HttpException& e)
		{
			response->write(extractErrorCode(e), e.what());
		}
	};

	server.resource["^/config/pairs/unsafe/.*$"]["GET"] = [provider](std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
	{
		/**
		 * This endpoint allows for SQL injection vulnerabilities in the URI path section,
		 * by utilizing hex symbols for special characters like single quotes and spaces.
		 * Experiment with the following curl command:
		 * curl -v --cacert server.crt -H "Content-Type: application/json"  'https://localhost:8080/config/pairs/unsafe/SOF-LON%27%20OR%20%27%27=%27'
		 */
		try
		{
			verifyHeaders(request->header);
			auto [username, password] = parseBasicAuthCredentials(request->header);
			
			const std::string& path = request->path_match[0];
			const std::string parsedPath = decodeHexSymbols(path);
			const std::string prefix = "/config/pairs/unsafe/";
			const std::string filter = parsedPath.substr(prefix.size());
			const size_t delimiter = filter.find('-');

			response->write(provider->getPairUnsafe(filter.substr(0, delimiter), filter.substr(delimiter + 1)));
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

		Utils::Options options(configPath);

		std::cout << "Done." << std::endl;
		std::cout << "Reading JSON schemas..." << std::endl;

		const std::string pairSchemaJson = loadFileToString(execPath + "../schemas/pair-schema.json");
		const std::string userSchemaJson = loadFileToString(execPath + "../schemas/user-schema.json");

		std::cout << "Done." << std::endl;

		auto provider = std::make_shared<ApiServer::ConfigProvider>(options.getMySqlHost(),
									 						  		options.getMySqlPort(),
									 						  		options.getMySqlUsername(),
									 						  		options.getMySqlPassword(),
															  		options.getMySqlDatabase());

		HttpsServer server(execPath + options.getCertificatePath(), execPath + options.getPrivateKeyPath());

		configure(server, options);
		addResources(server, provider, pairSchemaJson, userSchemaJson);

		std::thread serverThread([&server]()
		{
			server.start();
		});
		
		std::cout << "Server is running on port " << options.getPort() << "..." << std::endl;

		// VV: service goal
		/**
		 * DDoS mitigation: residential proxy, WAF, Load Balancer
		 * Sql Injection mitigation: user roles, request schemas, sql driver security
		 */

		serverThread.join();

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
