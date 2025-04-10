#pragma once
#include <string>
#include "Request.h"
#include "Response.h"

class Controller {
public:
    virtual void handleRequest(Request req, Response& resp) = 0;
    virtual std::string getControllerName() = 0;
    virtual bool validateRequest(Request req) = 0;
    virtual ~Controller() = default;
};
