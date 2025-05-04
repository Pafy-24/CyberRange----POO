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

std::string AuthController::getToken() const 
{
    return token;
}

std::string AuthController::getCurrentUser() const {
    return currentUser;
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
            if (status == "success") 
            {
                token = response.value("token", "");
                currentUser = response.value("username", "");

                std::cout << "[AuthController] Login successful. User: " << currentUser << "\n";
                // TODO: notificare UI, salveaza sesiunea
            }
            else 
            {
                std::string msg = response.value("message", "Login failed.");
                std::cerr << "[AuthController] Login failed: " << msg << "\n";
                // TODO: notificare UI cu mesaj de eroare
            }
        }

    }
    catch (const std::exception& e) {
        std::cerr << "[AuthController] Error parsing server response: " << e.what() << "\n";
    }
}