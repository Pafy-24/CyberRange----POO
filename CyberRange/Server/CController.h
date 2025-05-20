#pragma once
#include <string>
#include "Controller.h"
#include "Observable.h"
#include "Logger.h"
#include "Connection.h"

class CController : public Controller, public Observable {
protected:
    std::string controllerName;

public:
    CController(std::string name);
    bool validateRequest(const std::string& data, Connection* client, bool need2Blogged);
    void handleRequest(const std::string& data, Connection* client);
};
