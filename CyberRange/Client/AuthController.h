#pragma once
#include <string>
#include "CController.h"

class AuthController : public CController {
private:
    std::string token;

public:
    AuthController();
    bool login(std::string username, std::string password);
    bool validateToken();
    void logout();
    std::string getToken();
};
