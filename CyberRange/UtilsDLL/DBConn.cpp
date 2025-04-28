#include "pch.h"
#include "DBConn.h"
#include <iostream>
#include <regex>
#include <sstream>


DBConn::DBConn(const std::string& connStr)
    : dbSocket(nullptr), connectionString(connStr), port(3306),
    dbType("mysql"), connected(false), timeout(30000), tlsEnabled(false),
    serverRunning(false) {

    // Parse the connection string to extract host, port, etc.
    parseConnectionString();

    // Enable TLS by default for database connections
    enableTLS();
}


DBConn::~DBConn() {
    if (connected) {
        disconnect();
    }

    if (dbSocket) {
        delete dbSocket;
        dbSocket = nullptr;
    }
}

bool DBConn::parseConnectionString() {
    // Parse connection string to determine database type and extract components
    if (connectionString.find("postgresql://") == 0) {
        dbType = "postgresql";
        port = 5432;
    }
    else if (connectionString.find("oracle://") == 0) {
        dbType = "oracle";
        port = 1521;
    }
    else if (connectionString.find("sqlserver://") == 0) {
        dbType = "sqlserver";
        port = 1433;
    }
    else if (connectionString.find("mysql://") == 0 || connectionString.find("mariadb://") == 0) {
        dbType = "mysql";
        port = 3306;
    }

    // This is a simplified parser - a real implementation would be more robust
    size_t protocolEnd = connectionString.find("://");
    if (protocolEnd == std::string::npos) {
        std::cerr << "Invalid connection string format" << std::endl;
        return false;
    }

    size_t userPassEnd = connectionString.find('@', protocolEnd + 3);
    if (userPassEnd != std::string::npos) {
        std::string userPass = connectionString.substr(protocolEnd + 3, userPassEnd - (protocolEnd + 3));
        size_t colonPos = userPass.find(':');

        if (colonPos != std::string::npos) {
            username = userPass.substr(0, colonPos);
            password = userPass.substr(colonPos + 1);
        }
        else {
            username = userPass;
        }

        std::string hostPortDb = connectionString.substr(userPassEnd + 1);
        size_t portStart = hostPortDb.find(':');

        if (portStart != std::string::npos) {
            host = hostPortDb.substr(0, portStart);
            size_t dbStart = hostPortDb.find('/', portStart);

            if (dbStart != std::string::npos) {
                port = std::stoi(hostPortDb.substr(portStart + 1, dbStart - (portStart + 1)));
                database = hostPortDb.substr(dbStart + 1);
            }
            else {
                port = std::stoi(hostPortDb.substr(portStart + 1));
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

    return !host.empty();
}

bool DBConn::connect() {
    if (connected) {
        return true;
    }

    if (host.empty()) {
        std::cerr << "Invalid connection string, host is empty" << std::endl;
        return false;
    }

    std::cout << "Connecting to " << dbType << " database at " << host << ":" << port << std::endl;

    // Create a TCP socket connection to the database server
    dbSocket = new TCPSock(host, port);

    // Set timeout
    dbSocket->setTimeout(timeout);

    // Enable TLS if required
    if (tlsEnabled) {
        dbSocket->enableTLS();
    }

    // Connect to the database server
    if (!dbSocket->connect()) {
        std::cerr << "Failed to connect to database server at " << host << ":" << port << std::endl;
        delete dbSocket;
        dbSocket = nullptr;
        return false;
    }

    // Send authentication data (simplified)
    std::string authData = "AUTH " + username + " " + password + " " + database;
    int bytesSent = dbSocket->send(authData);

    if (bytesSent <= 0) {
        std::cerr << "Failed to send authentication data" << std::endl;
        dbSocket->disconnect();
        delete dbSocket;
        dbSocket = nullptr;
        return false;
    }

    // Wait for authentication response
    std::string response = dbSocket->receive();
    if (response.empty() || response.find("OK") == std::string::npos) {
        std::cerr << "Authentication failed: " << response << std::endl;
        dbSocket->disconnect();
        delete dbSocket;
        dbSocket = nullptr;
        return false;
    }

    connected = true;
    std::cout << "Connected to " << dbType << " database at " << host << ":" << port
        << " as user " << username << std::endl;

    return true;
}

bool DBConn::disconnect() {
    if (!connected) {
        return true;
    }

    if (dbSocket) {
        dbSocket->disconnect();
        delete dbSocket;
        dbSocket = nullptr;
    }

    std::cout << "Disconnected from database" << std::endl;
    connected = false;
    return true;
}

bool DBConn::isConnected() const {
    return connected && dbSocket && dbSocket->isConnected();
}

int DBConn::send(const std::string& query) {
    if (!connected || !dbSocket) {
        std::cerr << "Not connected to database" << std::endl;
        return -1;
    }

    std::string sanitizedQuery = query;
    // Sanitize query to prevent SQL injection
    if (!sanitizeQuery(sanitizedQuery)) {
        std::cerr << "Query failed sanitization checks" << std::endl;
        return -1;
    }

    std::cout << "Executing query: " << sanitizedQuery << std::endl;

    // Send the query to the database server
    return dbSocket->send(sanitizedQuery);
}

std::string DBConn::receive() {
    if (!connected || !dbSocket) {
        std::cerr << "Not connected to database" << std::endl;
        return "";
    }

    // Receive response from the database server
    return dbSocket->receive();
}

void DBConn::setTimeout(int ms) {
    timeout = ms;

    if (dbSocket) {
        dbSocket->setTimeout(ms);
    }

    std::cout << "Database timeout set to " << ms << " ms" << std::endl;
}

int DBConn::getTimeout() const {
    return timeout;
}

std::string DBConn::getAddress() const {
    return host;
}

int DBConn::getPort() const {
    return port;
}

bool DBConn::enableTLS() {
    tlsEnabled = true;

    if (dbSocket) {
        return dbSocket->enableTLS();
    }

    return true;
}

bool DBConn::isTLSEnabled() const {
    return tlsEnabled;
}

void DBConn::setDbType(const std::string& type) {
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




bool DBConn::bind(int port) {
    throw std::runtime_error("Operation not supported: DBConn does not support bind operation");
    return false;
}

bool DBConn::listen(int backlog) {
    throw std::runtime_error("Operation not supported: DBConn does not support listen operation");
    return false;
}

Connection* DBConn::accept() {
    throw std::runtime_error("Operation not supported: DBConn does not support accept operation");
    return nullptr;
}

bool DBConn::runServer() {
    throw std::runtime_error("Operation not supported: DBConn is a client connection only");
    return false;
}

void DBConn::stopServer() {
    throw std::runtime_error("Operation not supported: DBConn is a client connection only");
}

bool DBConn::isServerRunning() const {
    return false; // Always false for DBConn
}

