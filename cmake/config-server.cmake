include_directories(
    config-server/include
)

add_executable(configserver
    config-server/src/server.cpp
    config-server/src/config-provider.cpp
    utils/src/options.cpp
    utils/src/mysql-provider.cpp
)

set(BIN_DIR ${CMAKE_BINARY_DIR}/config-server/bin)

file(MAKE_DIRECTORY ${BIN_DIR})

set_target_properties(configserver
    PROPERTIES
    RUNTIME_OUTPUT_NAME server
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

target_link_libraries(configserver
    simple-web-server
    mysqlcppconn
    ValiJSON::valijson
)

configure_file(
    config-server/config.ini
    ${CMAKE_BINARY_DIR}/config-server/bin/
    COPYONLY
)

file(
    COPY ${CMAKE_SOURCE_DIR}/config-server/schemas/
    DESTINATION ${CMAKE_BINARY_DIR}/config-server/schemas
)

file(
    COPY ${CMAKE_SOURCE_DIR}/config-server/requests/
    DESTINATION ${CMAKE_BINARY_DIR}/config-server/requests
)

# Define paths for generated SSL files
set(SSL_DIR "${CMAKE_BINARY_DIR}/config-server/ssl")
set(SSL_KEY "${SSL_DIR}/server.key")
set(SSL_CSR "${SSL_DIR}/server.csr")
set(SSL_CERT "${SSL_DIR}/server.crt")

# Ensure the SSL directory exists
file(MAKE_DIRECTORY ${SSL_DIR})

# Command to generate a private key
add_custom_command(
    OUTPUT ${SSL_KEY}
    COMMAND openssl genrsa -out ${SSL_KEY} 2048
    COMMENT "Generating private key..."
)

# Command to generate a Certificate Signing Request (CSR)
add_custom_command(
    OUTPUT ${SSL_CSR}
    DEPENDS ${SSL_KEY}
    COMMAND openssl req -new -key ${SSL_KEY} -out ${SSL_CSR} -subj "/C=US/ST=State/L=City/O=Company/OU=Org/CN=localhost"
    COMMENT "Generating CSR..."
)

# Command to generate a self-signed certificate
add_custom_command(
    OUTPUT ${SSL_CERT}
    DEPENDS ${SSL_CSR}
    COMMAND openssl x509 -req -days 365 -in ${SSL_CSR} -signkey ${SSL_KEY} -out ${SSL_CERT}
    COMMENT "Generating self-signed SSL certificate..."
)

# Create a target that ensures SSL files are built before compiling the project
add_custom_target(generate_ssl ALL
    DEPENDS ${SSL_CERT}
)

# Ensure SSL certificates are available before building the main target
add_dependencies(configserver generate_ssl)
