#pragma once
#include <string>
#include "Controller.h"

class CController : public Controller {
protected:
    std::string controllerName;

public:
    explicit CController(const std::string& name);
    virtual ~CController() = default;

    std::string getControllerName() override;
    virtual void handleServerResponse(const std::string& response) override;
    virtual bool validateResponse(const std::string& response) override;
};