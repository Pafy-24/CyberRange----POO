#pragma once
#include <string>
#include "Controller.h"
#include "Response.h"

class CController : public Controller {
private:
    std::string controllerName;

public:
    CController(std::string name);
    void handleServerResponse(Response resp) override;
    std::string getControllerName() override;
    bool validateResponse(Response resp) override;
};
