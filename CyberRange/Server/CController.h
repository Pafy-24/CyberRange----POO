#pragma once
#include <string>
#include "Controller.h"
#include "Logger.h"
#include "Connection.h"

class CController : public Controller {
protected:
    std::string controllerName;
    Logger* logger;

public:
    CController(std::string name);
    bool validateRequest(const std::string& data, Connection* client);
    void handleRequest(const std::string& data, Connection* client);
};
