#pragma once
#include "TCPSock.h"
#include "DBConn.h"
#include <string>
#include <vector>
#include <thread>
#include <chrono>

class UTILS_API AdminConn : public virtual TCPSock, public virtual DBConn {
private:
    std::string adminKey;
    int privilege;
    bool secure;
    std::vector<Connection*> clientConnections;
    bool stopRequested;

protected:

    bool sanitizeQuery(std::string& query) override;
    bool verifyAdminCommand(const std::string& command);

public:
    AdminConn(int listenPort, const std::string& dbConnStr);

    AdminConn(std::unique_ptr<sf::TcpSocket> clientSock, const std::string& clientAddr, int clientPort, const std::string& dbConnStr);

    ~AdminConn() override;

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
    std::string getType() const override { return "Admin"; }
    void handleRequest(const std::string& data, Connection* client = nullptr) override;

    bool bind(int port) override;
    bool listen(int backlog = 5) override;
    Connection* accept() override;
    bool runServer() ;
    void stopServer() override;
    bool isServerRunning() const override;
    void setCertificates(const std::string& cert, const std::string& key) override;

    void setAdminKey(const std::string& key);
    bool verifyPrivilege(int level) const;
    void enableSecureMode();
    void disableSecureMode();
    bool isSecureModeEnabled() const { return secure; }
    int getPrivilegeLevel() const { return privilege; }

    bool processClientRequest();
    bool forwardToDBServer(const std::string& request);
    bool sendResponseToClient(const std::string& response);
};