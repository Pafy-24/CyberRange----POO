#pragma once
#include "DBConn.h"
#include "TCPSock.h"
#include <string>
#include <vector>
#include <thread>
#include <chrono>

class UTILS_API AdminConn : public Connection {
private:
    // Connection to the database server
    DBConn* dbConnection;

    // Connection to the client
    TCPSock* clientSocket;

    // Server socket for accepting client connections
    TCPSock* serverSocket;

    std::string adminKey;
    int privilege;
    bool secure;
    bool connected;
    int port;
    int timeout;
    bool tlsEnabled;
    bool serverRunning;
    std::vector<Connection*> clientConnections;
    bool stopRequested;

protected:
    bool sanitizeQuery(std::string& query);
    bool verifyAdminCommand(const std::string& command);

public:
    // Constructor for server mode (listens for client connections)
    AdminConn(int listenPort, const std::string& dbConnStr);

    // Constructor for direct client connection
    AdminConn(TCPSock* clientSock, const std::string& dbConnStr);

    virtual ~AdminConn();

    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    int send(const std::string& data) override;
    std::string receive() override;
    std::string getAddress() const override;
    int getPort() const override;
    bool enableTLS() override;
    bool isTLSEnabled() const override;
    void setTimeout(int ms) override;
    int getTimeout() const override;
    bool bind(int port) override;
    bool listen(int backlog = 5) override;
    Connection* accept() override;

    // Server operations
    bool runServer() override;
    void stopServer() override;
    bool isServerRunning() const override;

    // Admin specific methods
    void setAdminKey(const std::string& key);
    bool verifyPrivilege(int level) const;
    void enableSecureMode();
    void disableSecureMode();
    bool isSecureModeEnabled() const { return secure; }
    int getPrivilegeLevel() const { return privilege; }

    // Process client requests and forward to DB server
    bool processClientRequest();
    bool forwardToDBServer(const std::string& request);
    bool sendResponseToClient(const std::string& response);
};