#include "DBConn.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>

DBConn::DBConn(std::string connStr)
    : connectionString(connStr), connected(false), dbType("mysql"),
    timeout(30000), dbHandle(nullptr), sslContext(nullptr), tlsEnabled(false) {
    // Parse connection string to determine database type
    if (connStr.find("postgresql://") == 0) {
        dbType = "postgresql";
    }
    else if (connStr.find("oracle://") == 0) {
        dbType = "oracle";
    }
    else if (connStr.find("sqlserver://") == 0) {
        dbType = "sqlserver";
    }

    // Enable TLS by default for database connections
    enableTLS();
}

DBConn::~DBConn() {
    if (connected) {
        disconnect();
    }

    if (sslContext) {
        SSL_CTX_free(static_cast<SSL_CTX*>(sslContext));
        sslContext = nullptr;
    }
}

bool DBConn::connect() {
    if (connected) {
        return true;
    }

    std::cout << "Connecting to database using " << connectionString << std::endl;

    // Parse connection string to extract components
    std::string host, port, database, user, password;

    // This is a simplified parser - a real implementation would be more robust
    size_t protocolEnd = connectionString.find("://");
    if (protocolEnd != std::string::npos) {
        size_t userPassEnd = connectionString.find('@', protocolEnd + 3);

        if (userPassEnd != std::string::npos) {
            std::string userPass = connectionString.substr(protocolEnd + 3, userPassEnd - (protocolEnd + 3));
            size_t colonPos = userPass.find(':');

            if (colonPos != std::string::npos) {
                user = userPass.substr(0, colonPos);
                password = userPass.substr(colonPos + 1);
            }
            else {
                user = userPass;
            }

            std::string hostPortDb = connectionString.substr(userPassEnd + 1);
            size_t portStart = hostPortDb.find(':');

            if (portStart != std::string::npos) {
                host = hostPortDb.substr(0, portStart);
                size_t dbStart = hostPortDb.find('/', portStart);

                if (dbStart != std::string::npos) {
                    port = hostPortDb.substr(portStart + 1, dbStart - (portStart + 1));
                    database = hostPortDb.substr(dbStart + 1);
                }
                else {
                    port = hostPortDb.substr(portStart + 1);
                }
            }
            else {
                size_t dbStart = hostPortDb.find('/');

                if (dbStart != std::string::npos) {
                    host = hostPortDb.substr(0, dbStart);
                    database = hostPortDb.substr(dbStart + 1);
                }
                else {
                    host = hostPortDb;
                }
            }
        }
    }

    // In a real implementation, we would connect to the actual database here
    // using the appropriate library (libpq for PostgreSQL, MySQL C API, etc.)

    // For now, we'll just simulate a connection
    if (host.empty()) {
        std::cerr << "Invalid connection string format" << std::endl;
        return false;
    }

    // Initialize TLS if enabled
    if (tlsEnabled) {
        // Initialize OpenSSL
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();

        // Create SSL context
        const SSL_METHOD* method = TLS_client_method();
        SSL_CTX* ctx = SSL_CTX_new(method);

        if (!ctx) {
            std::cerr << "Failed to create SSL context for database connection" << std::endl;
            ERR_print_errors_fp(stderr);
            return false;
        }

        sslContext = ctx;

        std::cout << "TLS enabled for database connection" << std::endl;
    }

    // Simulating connection to database
    std::cout << "Connected to " << dbType << " database at " << host;
    if (!port.empty()) {
        std::cout << ":" << port;
    }
    std::cout << " as user " << user << std::endl;

    connected = true;
    return true;
}

bool DBConn::disconnect() {
    if (!connected) {
        return true;
    }

    // In a real implementation, we would close the database connection here

    std::cout << "Disconnected from database" << std::endl;
    connected = false;
    return true;
}

bool DBConn::isConnected() {
    return connected;
}

int DBConn::send(std::string query) {
    if (!connected) {
        std::cerr << "Not connected to database" << std::endl;
        return -1;
    }

    // Sanitize query to prevent SQL injection
    if (!sanitizeQuery(query)) {
        std::cerr << "Query failed sanitization checks" << std::endl;
        return -1;
    }

    // In a real implementation, we would execute the query here

    std::cout << "Executing query: " << query << std::endl;

    // Simulate query execution
    // Return number of affected rows (just a placeholder)
    return 1;
}

std::string DBConn::receive() {
    if (!connected) {
        std::cerr << "Not connected to database" << std::endl;
        return "";
    }

    // In a real implementation, we would fetch results here

    // Simulate fetching results
    std::stringstream result;
    result << "{ \"result\": \"success\", \"rows\": [] }";

    return result.str();
}

void DBConn::setTimeout(int ms) {
    timeout = ms;

    // In a real implementation, we would set the database client timeout here

    std::cout << "Database timeout set to " << ms << " ms" << std::endl;
}

std::string DBConn::getAddress() {
    // Extract host:port from connection string
    size_t protocolEnd = connectionString.find("://");
    if (protocolEnd != std::string::npos) {
        size_t userPassEnd = connectionString.find('@', protocolEnd + 3);

        if (userPassEnd != std::string::npos) {
            std::string hostPortDb = connectionString.substr(userPassEnd + 1);
            size_t dbStart = hostPortDb.find('/');

            if (dbStart != std::string::npos) {
                return hostPortDb.substr(0, dbStart);
            }
            else {
                return hostPortDb;
            }
        }
    }

    return "";
}

bool DBConn::enableTLS() {
    if (connected) {
        std::cerr << "Cannot enable TLS on already connected database" << std::endl;
        return false;
    }

    tlsEnabled = true;
    return true;
}

bool DBConn::isTLSEnabled() {
    return tlsEnabled;
}

void DBConn::setDbType(std::string type) {
    if (!connected) {
        dbType = type;
    }
    else {
        std::cerr << "Cannot change database type while connected" << std::endl;
    }
}

bool DBConn::sanitizeQuery(std::string& query) {
    // Basic SQL injection prevention
    // In a real implementation, this would be more sophisticated
    // and use prepared statements where possible

    // Check for typical SQL injection patterns
    std::regex sqlInjectionPattern(
        "('\\s*OR\\s*'\\s*'\\s*=\\s*')|"     // 'OR''='
        "(--\\s)|"                           // SQL comment
        "(;\\s*DROP\\s+TABLE)|"              // DROP TABLE
        "(;\\s*DELETE\\s+FROM)|"             // DELETE FROM
        "(UNION\\s+SELECT)|"                 // UNION SELECT
        "(EXEC\\s+xp_)|"                     // SQL Server stored procedures
        "(INTO\\s+OUTFILE)"                  // MySQL file operations
    );

    if (std::regex_search(query, sqlInjectionPattern)) {
        std::cerr << "Potential SQL injection detected" << std::endl;
        return false;
    }

    return true;
}