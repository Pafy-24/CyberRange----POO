#pragma once
#include <string>
#include <vector>
#include "Connection.h"
#include "Controller.h"
#include "ChallMng.h"
#include "Loader.h"

class ServerMng {
private:
    std::vector<Connection*> connections;
    std::vector<Controller*> controllers;
    std::vector<ChallMng*> challMngs;
    Loader* loader;
    bool isRunning;
    int port;
    std::string serverAddress;

public:
    ServerMng(int port, std::string address);
    void start();
    void stop();
    void addConnection(Connection* conn);
    void removeConnection(Connection* conn);
    void registerController(Controller* ctrl);
    void processRequests();
    Connection* getConnection(int id);
    ChallMng* getChallMng(std::string contestId);
};
