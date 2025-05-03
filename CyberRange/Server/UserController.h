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
    void handleRequest(const std::string& data, Connection* client) override;
    void loadUser(const std::string& userId);
    void unloadUser(const std::string& userId);
	DBController* getDB() const { return dbController; }
};