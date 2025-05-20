#pragma once
#include "CController.h"
#include "json.hpp"
#include <string>


class Connection;

class UserController : public CController {
public:
    UserController();

    void Login(const json& data, Connection* client);
    void Register(const json& data, Connection* client);
    void Update(const json& data, Connection* client);
    void Logout(const json& data, Connection* client);
    void Delete(const json& data, Connection* client);
    void sendAllUsers(Connection* client);

    void handleRequest(const std::string& data, Connection* client) override;
};