#pragma once
#include <string>
#include "CController.h"
#include <QObject>
#include "json.hpp"
using json = nlohmann::json;

class AuthController : public QObject, public CController
{
    Q_OBJECT
public:
    AuthController();

    void requestLogin(const std::string& username, const std::string& password);
    void requestRegister(const std::string& username, const std::string& password, const std::string& email);
    void requestUpdate(const std::string& username, const std::string& old, const std::string& password, const std::string& email);
    void requestDelete();
    void requestLogout();

    std::string getToken() const;
    std::string getCurrentUser() const;
    bool isAuthenticated() const;
    int getRole() const;

    void requestUserList();

    void handleServerResponse(const std::string& responseStr) override;

signals:
    void loginSucceeded();
    void loginFailed(const QString& message);
    void registerSucceeded();
    void registerFailed(const QString& message);
    void updateSucceeded();
    void updateFailed(const QString& message);
    void deleteSucceeded();
    void deleteFailed(const QString& message);
    void logoutSucceeded();
    void logoutFailed(const QString& message);
    void usersReceived(const json& users);

private:
    std::string token;
    std::string currentUser;
    int role;
};