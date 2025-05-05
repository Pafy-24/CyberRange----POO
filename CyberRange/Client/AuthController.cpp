#include "AuthController.h"
#include "ClientMng.h"
#include <QTimer>
#include <QDebug>
#include <json.hpp>

using json = nlohmann::json;

AuthController::AuthController()
    : CController("AuthController") // Numele controllerului pentru identificare 
{}

void AuthController::requestLogin(const std::string& username, const std::string& password)
{
    json req = {
        {"controller", "UserController"},
        {"action", "login"},
        {"payload", {
            {"username", username},
            {"password", password}
        }}
    };

    ClientMng::getInstance()->sendRequest(req.dump());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ClientMng::getInstance()->receiveResponse();

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

                int r = response["role"];
                switch (r)
                {
                case 1:
                    role = "student";break;
                case 5:
                    role="writer";break;
                case 10:
                    role = "admin";break;
                default:
                    role = "student";
                    break;
                }

                qDebug() << "[AuthController] Login reușit. Utilizator:" << QString::fromStdString(currentUser);
                emit loginSucceeded(); // semnal pentru UI
            }
            else 
            {
                QString msg = QString::fromStdString(response.value("message", "Login failed."));
                qWarning() << "[AuthController] Login eșuat:" << msg;
                emit loginFailed(msg); // semnal pentru UI
            }
        }
    }
    catch (const std::exception& e) 
    {
        qCritical() << "[AuthController] Eroare parsare răspuns server:" << e.what();
        emit loginFailed("Eroare internă în timpul autentificării.");
    }
}

void AuthController::requestRegister(const std::string& username, const std::string& password, const std::string& email)
{
    json req = {
        {"controller", "UserController"},
        {"action", "register"},
        {"payload", {
            {"username", username},
            {"password", password},
            {"email", email}
        }}
    };

    ClientMng::getInstance()->sendRequest(req.dump());
}

void AuthController::requestLogout()
{
    if (token.empty()) {
        qWarning() << "[AuthController] Nu există sesiune activă pentru logout.";
        emit loginFailed("Nu ești autentificat.");
        return;
    }

    json req = {
        {"controller", "UserController"},
        {"action", "logout"},
        {"token", token}
    };

    ClientMng::getInstance()->sendRequest(req.dump());
}

std::string AuthController::getToken() const 
{
    return token;
}

std::string AuthController::getCurrentUser() const 
{
    return currentUser;
}

bool AuthController::isAuthenticated() const 
{
    return !token.empty();
}

std::string AuthController::getRole() const
{
    return role;
}
