#pragma once
#include <string>
#include "Response.h"

class Controller {
public:
    virtual void handleServerResponse(Response resp) = 0;
    virtual std::string getControllerName() = 0;
    virtual bool validateResponse(Response resp) = 0;
    virtual ~Controller() = default;
};
