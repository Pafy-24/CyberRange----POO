#pragma once
#include <string>
#include "Controller.h"
#include "Logger.h"

class CController : public Controller {
private:
    Logger* logger;
protected:
    std::string controllerName;

public:
    CController(std::string name);
};
