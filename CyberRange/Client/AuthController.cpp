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
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ClientMng::getInstance()->receiveResponse();
}

void AuthController::requestUpdate(const std::string& username, const std::string& password, const std::string& email)
{
    json req = {
        {"controller", "UserController"},
        {"action", "update"},
		{"token", token},
        {"payload", {
            {"username", username},
            {"password", password},
            {"email", email}
        }}
    };

    ClientMng::getInstance()->sendRequest(req.dump());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ClientMng::getInstance()->receiveResponse();
}

void AuthController::requestDelete()
{

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

                role = response["role"];
                /*switch (r)
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
                }*/

                qDebug() << "[AuthController] Login succesfully done. User:" << QString::fromStdString(currentUser);
                emit loginSucceeded(); // semnal pentru UI
            }
            else 
            {
                QString msg = QString::fromStdString(response.value("message", "Login failed."));
                qWarning() << "[AuthController] Login failed:" << msg;
                emit loginFailed(msg); // semnal pentru UI
            }
        }
		else if (action == "register")
		{
			if (status == "success")
			{
				qDebug() << "[AuthController] Registered successfully.";
			}
			else
			{
				QString msg = QString::fromStdString(response.value("message", "Registration failed."));
				qWarning() << "[AuthController] Înregistrare eșuată:" << msg;
				//emit loginFailed(msg); // semnal pentru UI
			}
		}
		else if (action == "update")
		{
			if (status == "success")
			{
				qDebug() << "[AuthController] User updated successfully.";
			}
			else
			{
				QString msg = QString::fromStdString(response.value("message", "Update failed."));
				qWarning() << "[AuthController] Actualizare eșuată:" << msg;
			}
		}
        else if (action == "logout")
        {
            if (status == "success")
            {
                token.clear();
                currentUser.clear();
                role = 0;
                qDebug() << "[AuthController] Logout reușit.";
            }
            else
            {
                QString msg = QString::fromStdString(response.value("message", "Logout failed."));
                qWarning() << "[AuthController] Logout eșuat:" << msg;
            }
        }
    }
    catch (const std::exception& e) 
    {
        qCritical() << "[AuthController] Eroare parsare răspuns server:" << e.what();
        emit loginFailed("Eroare internă în timpul autentificării.");
    }
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

int AuthController::getRole() const
{
    return role;
}
