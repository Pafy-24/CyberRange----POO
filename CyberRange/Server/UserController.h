#pragma once
#include "CController.h"
#include "User.h"
#include "DBController.h"


class UserController : public CController {
private:
    DBController* dbController;
    std::map<std::string, User*> users;

public:
    UserController(DBController* dbCtrl);
    void Login(const json& data, Connection* client);
    void Register(const json& data, Connection* client);
    void Update(const json& data, Connection* client);
    void Delete(const json& data, Connection* client);
	void Logout(const json& data, Connection* client);

    void handleRequest(const std::string& data, Connection* client) override;
	DBController* getDB() const { return dbController; }
};