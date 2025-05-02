#pragma once
#include <string>

class Controller { // interfata de controller pentru client
public:
    virtual void handleServerResponse(const std::string& response) = 0;
    virtual std::string getControllerName() = 0;
    virtual bool validateResponse(const std::string& response) = 0;
    virtual ~Controller() = default;
};