#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "Connection.h"
#include "ChallMng.h"
#include "Loader.h"
#include "User.h"

class ServerMng {
private:
    static ServerMng* instance;
    static std::mutex instanceMutex;

    std::vector<Connection*> connections;
    std::mutex connectionsMutex;
    std::map<std::string, Controller*> controllers;
    std::vector<ChallMng*> challMngs;
    std::vector<User*> users;
    Loader* loader;
    bool isRunning;
    int port;
    std::string serverAddress;

    ServerMng(int port, std::string address);
    void runTCPServer(int port, bool useTLS);
    void runUDPServer(int port);
    void runDownloadServer(int port);

public:
    static ServerMng* getInstance(int port = 1337, const std::string& address = "0.0.0.0");
    ~ServerMng();

    ServerMng(const ServerMng&) = delete;
    ServerMng& operator=(const ServerMng&) = delete;

    void start();
    void stop();

    void addConnection(Connection* conn);
    void removeConnection(Connection* conn);

    void processRequests();

    void attachController(std::string name, Controller* ctrl);
    void removeController(std::string name);

    Connection* getConnection(int id);
    ChallMng* getChallMng(std::string contestId);
};