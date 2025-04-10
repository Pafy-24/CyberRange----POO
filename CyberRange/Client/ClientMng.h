#pragma once
#include <string>
#include <vector>
#include "Connection.h"
#include "Loader.h"
#include "Controller.h"
#include "Request.h"
#include "Response.h"

class ClientMng {
private:
    Connection* serverConn;
    Loader* loader;
    std::vector<Controller*> controllers;
    bool connected;
    std::string serverAddress;
    int serverPort;

public:
    ClientMng(std::string address, int port);
    bool connect();
    void disconnect();
    void sendRequest(Request req);
    Response receiveResponse();
    void registerController(Controller* ctrl);
    Controller* getController(std::string name);
};
