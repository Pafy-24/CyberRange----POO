#include "UsersFactory.h"
#include "UserController.h"
#include "ServerMng.h"
#include "json.hpp"
#include "CustomSerial.h"
#include <iostream>

using json = nlohmann::json;

UserController::UserController(DBController* dbCtrl)
    : CController("UserController"), dbController(dbCtrl) {
}

void UserController::Login(const json& data, Connection* client)
{
}

void UserController::Register(const json& data, Connection* client)
{
}

void UserController::Update(const json& data, Connection* client)
{
}

void UserController::Delete(const json& data, Connection* client)
{
}

void UserController::Logout(const json& data, Connection* client)
{
}

void UserController::handleRequest(const std::string& data, Connection* client) {
    if (!validateRequest(data, client)) {
        return;
    }

    try {
        json j = json::parse(data)[0];
        std::string action = j["action"].get<std::string>();

        if (action == "LOGIN") {
			Login(j, client);

            /*std::string username = j["username"].get<std::string>();
            std::string password = j["password"].get<std::string>();

            std::string query = "SELECT * FROM Users WHERE Username = '" + username +
                "' AND PasswordHash = '" + password + "'";
            auto results = dbController->executeQuery(query);

            if (!results.empty()) 
            {
                json userData = {
                    {"userId", results[0]["userId"]},
					{"username", username},
					{"role", results[0]["role"]},
					{"lastActive", results[0]["lastActive"]}
                };
                std::string token = CustomSerial::encodeJWT(userData.dump());

				ServerMng::getInstance()->addToken(token);

                client->send(json{ {"status", "success"}, {"token", token} }.dump());
                logger->log("User logged in: " + username);
            }
            else 
            {
                client->send("ERROR: Invalid credentials");
            }*/
        }
        else if (action == "REGISTER") {
            std::string userId = j["userId"].get<std::string>();
            loadUser(userId);

            if (users.find(userId) != users.end()) {
                json response = {
                    {"status", "success"},
                    {"userId", userId},
                    {"username", users[userId]->GetUsername()}
                };
                client->send(response.dump());
            }
            else {
                client->send("ERROR: User not found");
            }
        }
        else if (action == "UPDATE") {
            Update(data, client);
        }
        else if (action == "DELETE") {
            Delete(data, client);
        }
        else if (action == "LOGOUT") {
            Logout(data, client);
        }
        else {
            client->send("ERROR: Invalid action");
        }
    }
    catch (const std::exception& e) {
        client->send("ERROR: Request processing failed");
        logger->log("UserController error: " + std::string(e.what()));
    }
}

void UserController::loadUser(const std::string& userId) {
    if (users.find(userId) != users.end()) {
        return;
    }

    std::string query = "SELECT * FROM Users WHERE UserID = '" + userId + "'";
    auto results = dbController->executeQuery(query);

    if (!results.empty()) {
		User* user = UsersFactory::CreateUser(UserType::COMMON, results[0]["username"], results[0]["userId"]).release();
        users[userId] = user;
        logger->log("Loaded user: " + userId);
    }
}

void UserController::unloadUser(const std::string& userId) {
    auto it = users.find(userId);
    if (it != users.end()) {
        std::string query = "UPDATE Users SET lastActive = GETDATE() WHERE UserID = '" + userId + "'";
        dbController->executeUpdate(query);
        delete it->second;
        users.erase(it);
        logger->log("Unloaded user: " + userId);
    }
}