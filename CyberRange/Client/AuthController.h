#pragma once
#include <string>
#include "CController.h"
#include <QObject>

class AuthController : public QObject, public CController 
{
    Q_OBJECT
public:
    AuthController();

    void requestLogin(const std::string& username, const std::string& password);
    void requestRegister(const std::string& username, const std::string& password, const std::string& email);
	void requestLogout();
    std::string getToken() const;
    std::string getCurrentUser() const;
    bool isAuthenticated() const;
	int getRole() const;
    void handleServerResponse(const std::string& responseStr) override;

signals:
    void loginSucceeded();
    void loginFailed(const QString& message);

private:
    std::string token;
    std::string currentUser;
	int role;
};