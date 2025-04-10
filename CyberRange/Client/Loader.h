#pragma once
#include <string>
#include <map>
#include "Controller.h"

class Loader {
private:
    std::map<std::string, Controller*> controllerMap;

public:
    Loader();
    void loadControllers();
    Controller* getController(std::string name);
};
