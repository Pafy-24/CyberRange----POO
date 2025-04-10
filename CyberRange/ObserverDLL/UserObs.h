#pragma once
#include <string>
#include <map>
#include "CObs.h"
#include "Logger.h"

class UserObs : public CObs {
private:
    std::map<std::string, int> userActions;

public:
    UserObs(Logger& logger);
    void trackLogin(std::string userId);
    void trackLogout(std::string userId);
    void trackPasswordChange(std::string userId);
    std::map<std::string, int> getUserStats();
};
