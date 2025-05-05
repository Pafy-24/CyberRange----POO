#include "AuthController.h"
#include "ClientMng.h"
#include <json.hpp>
#include <iostream>

using json = nlohmann::json;

AuthController::AuthController() : CController("Auth") {}

void AuthController::requestLogin(const std::string& username, const std::string& password) 
{
    json req = {
        {"controller", "Auth"},
        {"action", "login"},
        {"payload", {
            {"username", username},
            {"password", password}
        }}
    };
    ClientMng::getInstance()->sendRequest(req.dump());
}

void AuthController::requestRegister(const std::string& username, const std::string& password, const std::string& email)
{
    json req = {
        {"controller", "Auth"},
        {"action", "register"},
        {"payload", {
            {"username", username},
            {"password", password},
            {"email", email}
        }}
    };
    ClientMng::getInstance()->sendRequest(req.dump());
}


std::string AuthController::getToken() const 
{
    return token;
}

std::string AuthController::getCurrentUser() const {
    return currentUser;
}

std::string AuthController::getRole() const
{
    return role;
}

void AuthController::handleServerResponse(const std::string& responseStr) 
{
    try {
        json response = json::parse(responseStr);

        if (!response.contains("action") || !response.contains("status"))
            return;

        std::string action = response["action"];
        std::string status = response["status"];

        if (action == "login") 
        {
            if (status == "success") {
                token = response.value("token", "");
                currentUser = response.value("username", "");
                role = response.value("role", "common");  // <- rol aici

                std::cout << "[AuthController] Login successful. User: " << currentUser << " (role: " << role << ")\n";
            }
            else 
            {
                std::string msg = response.value("message", "Login failed.");
                std::cerr << "[AuthController] Login failed: " << msg << "\n";
                // TODO: notificare UI cu mesaj de eroare
            }
        }
        else if (action == "register") 
        {
            if (status == "success") 
            {
                token = response.value("token", "");
                currentUser = response.value("username", "");
                std::cout << "[AuthController] Register successful. User: " << currentUser << "\n";
            }
            else 
            {
                std::string msg = response.value("message", "Register failed.");
                std::cerr << "[AuthController] Register failed: " << msg << "\n";
            }
        }

    }
    catch (const std::exception& e) {
        std::cerr << "[AuthController] Error parsing server response: " << e.what() << "\n";
    }
}

bool AuthController::isAuthenticated() const
{
    return !token.empty();
}
