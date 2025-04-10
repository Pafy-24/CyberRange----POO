#pragma once
#include <string>
#include "Controller.h"
#include "Request.h"
#include "Response.h"
#include "Logger.h"

class CController : public Controller {
private:
    Request request;
    Response response;
    Logger* logger;
protected:
    std::string controllerName;

public:
    CController(std::string name);
    void setRequest(Request req);
    Response getResponse();
    void process();
    std::string getControllerName() override;
};
