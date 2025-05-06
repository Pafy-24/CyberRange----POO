#include "AuthController.h"
#include "ClientMng.h"
#include <QTimer>
#include <QDebug>
#include <json.hpp>

using json = nlohmann::json;

AuthController::AuthController()
    : CController("AuthController"), role(-1) // Controller name for identification 
{
}

void AuthController::requestLogin(const std::string& username, const std::string& password)
{
    if (username.empty() || password.empty()) {
        emit loginFailed("Username and password cannot be empty");
        return;
    }

    json req = {
        {"controller", "UserController"},
        {"action", "login"},
        {"payload", {
            {"username", username},
            {"password", password}
        }}
    };

    try {
        ClientMng::getInstance()->sendRequest(req.dump());

        QTimer::singleShot(500, [this]() {
            if (ClientMng::getInstance()->isConnected()) {
                ClientMng::getInstance()->receiveResponse();
            }
            else {
                emit loginFailed("Connection to server lost");
            }
            });
    }
    catch (const std::exception& e) {
        emit loginFailed(QString("Request error: %1").arg(e.what()));
    }
}

void AuthController::requestRegister(const std::string& username, const std::string& password, const std::string& email)
{
    if (username.empty() || password.empty() || email.empty()) {
        emit registerFailed("All fields are required");
        return;
    }

    json req = {
        {"controller", "UserController"},
        {"action", "register"},
        {"payload", {
            {"username", username},
            {"password", password},
            {"email", email}
        }}
    };

    try {
        ClientMng::getInstance()->sendRequest(req.dump());
        // Use QTimer for async behavior instead of blocking with sleep
        QTimer::singleShot(500, [this]() {
            if (ClientMng::getInstance()->isConnected()) {
                ClientMng::getInstance()->receiveResponse();
            }
            else {
                emit registerFailed("Connection to server lost");
            }
            });
    }
    catch (const std::exception& e) {
        emit registerFailed(QString("Request error: %1").arg(e.what()));
    }
}

void AuthController::requestUpdate(const std::string& username, const std::string& old, const std::string& password, const std::string& email)
{

    json req = {
        {"controller", "UserController"},
        {"action", "update"},
        {"token", token},
        {"payload", {
            {"username", username},
            {"password", password},
            {"oldPassword", old},
            {"email", email}
        }}
    };

    try {
        ClientMng::getInstance()->sendRequest(req.dump());
        // Use QTimer for async behavior instead of blocking with sleep
        QTimer::singleShot(500, [this]() {
            if (ClientMng::getInstance()->isConnected()) {
                ClientMng::getInstance()->receiveResponse();
            }
            else {
                emit updateFailed("Connection to server lost");
            }
            });
    }
    catch (const std::exception& e) {
        emit updateFailed(QString("Request error: %1").arg(e.what()));
    }
}

void AuthController::requestDelete()
{
    if (token.empty()) {
        emit deleteFailed("No active session for account deletion");
        return;
    }

    json req = {
        {"controller", "UserController"},
        {"action", "delete"},
        {"token", token}
    };

    try {
        ClientMng::getInstance()->sendRequest(req.dump());
        // Use QTimer for async behavior instead of blocking with sleep
        QTimer::singleShot(500, [this]() {
            if (ClientMng::getInstance()->isConnected()) {
                ClientMng::getInstance()->receiveResponse();
            }
            else {
                emit deleteFailed("Connection to server lost");
            }
            });
    }
    catch (const std::exception& e) {
        emit deleteFailed(QString("Request error: %1").arg(e.what()));
    }
}

void AuthController::handleServerResponse(const std::string& responseStr)
{
    try {
        json response = json::parse(responseStr);


        if (response.contains("token")) { token = response["token"]; }
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
                role = response.value("role", 1); // Default to regular user role if not specified

                qDebug() << "[AuthController] Login successfully done. User:" << QString::fromStdString(currentUser);
                emit loginSucceeded(); // Signal for UI
            }
            else
            {
                QString msg = QString::fromStdString(response.value("message", "Login failed."));
                qWarning() << "[AuthController] Login failed:" << msg;
                emit loginFailed(msg); // Signal for UI
            }
        }
        else if (action == "register")
        {
            if (status == "success")
            {
                qDebug() << "[AuthController] Registered successfully.";
                emit registerSucceeded();
            }
            else
            {
                QString msg = QString::fromStdString(response.value("message", "Registration failed."));
                qWarning() << "[AuthController] Registration failed:" << msg;
                emit registerFailed(msg);
            }
        }
        else if (action == "update")
        {
            if (status == "success")
            {
                qDebug() << "[AuthController] User updated successfully.";
                emit updateSucceeded();
            }
            else
            {
                QString msg = QString::fromStdString(response.value("message", "Update failed."));
                qWarning() << "[AuthController] Update failed:" << msg;
                emit updateFailed(msg);
            }
        }
        else if (action == "delete")
        {
            if (status == "success")
            {
                qDebug() << "[AuthController] Account deleted successfully.";
                emit deleteSucceeded();
                // Clear credentials
                token.clear();
                currentUser.clear();
                role = -1;
                // Trigger logout after successful deletion
                emit logoutSucceeded();
            }
            else
            {
                QString msg = QString::fromStdString(response.value("message", "Account deletion failed."));
                qWarning() << "[AuthController] Account deletion failed:" << msg;
                emit deleteFailed(msg);
            }
        }
        else if (action == "logout")
        {
            if (status == "success")
            {
                token.clear();
                currentUser.clear();
                role = -1;
                qDebug() << "[AuthController] Logout successful.";
                emit logoutSucceeded();
            }
            else
            {
                QString msg = QString::fromStdString(response.value("message", "Logout failed."));
                qWarning() << "[AuthController] Logout failed:" << msg;
                emit logoutFailed(msg);
            }
        }
    }
    catch (const std::exception& e)
    {
        qCritical() << "[AuthController] Error parsing server response:" << e.what();
        emit loginFailed("Internal error during authentication.");
    }
}

void AuthController::requestLogout()
{
    if (token.empty()) {
        emit logoutFailed("No active session for logout");
        return;
    }

    json req = {
        {"controller", "UserController"},
        {"action", "logout"},
        {"token", token}
    };

    try {
        ClientMng::getInstance()->sendRequest(req.dump());
        // Use QTimer for async behavior instead of blocking with sleep
        QTimer::singleShot(500, [this]() {
            if (ClientMng::getInstance()->isConnected()) {
                ClientMng::getInstance()->receiveResponse();
            }
            else {
                emit logoutFailed("Connection to server lost");
            }
            });
    }
    catch (const std::exception& e) {
        emit logoutFailed(QString("Request error: %1").arg(e.what()));
    }
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