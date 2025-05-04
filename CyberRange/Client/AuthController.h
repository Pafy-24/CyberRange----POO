#pragma once
#include <string>
#include "CController.h"

class AuthController : public CController {
private:
    std::string token;
	std::string currentUser;
public:
	AuthController();
    void requestLogin(const std::string& username, const std::string& password);
    std::string getToken() const;
    std::string getCurrentUser() const;

    void handleServerResponse(const std::string& responseStr) override;
};