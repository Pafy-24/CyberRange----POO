#pragma once
#include <string>
#include "Controller.h"

class CController : public Controller {
private:
    std::string controllerName;

public:
    CController(std::string name);
    void handleServerResponse(const std::string& response) override;
    std::string getControllerName() override;
    bool validateResponse(const std::string& response) override;
};