#pragma once
#include <string>
#include "CController.h"

class AuthController : public CController {
private:
    std::string token;
	std::string currentUser; std::string role;
public:
	AuthController();
    void requestLogin(const std::string& username, const std::string& password);
    void requestRegister(const std::string& username, const std::string& password, const std::string& email);
    std::string getToken() const;
    std::string getCurrentUser() const;
    std::string getRole() const;

    void handleServerResponse(const std::string& responseStr) override;
    bool isAuthenticated() const;
};