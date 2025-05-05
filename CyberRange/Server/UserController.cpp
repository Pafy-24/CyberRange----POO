#include "UsersFactory.h"
#include "UserController.h"
#include "ServerMng.h"
#include "CustomSerial.h"
#include <iostream>

using json = nlohmann::json;

UserController::UserController(DBController* dbCtrl)
    : CController("UserController"), dbController(dbCtrl) {
}

void UserController::Login(const json& data, Connection* client)
{
    std::string username = data["username"];
    std::string password = data["password"];

    std::string query = "SELECT * FROM Users WHERE username = '" + username +
        "' AND password = '" + password + "'";

    auto results = dbController->executeQuery(query);

    if (!results.empty())
    {
        // construim token
        json userData =
        {
            {"userId", results[0]["userId"]},
            {"username", username},
            {"role", results[0]["role"]},
            {"lastActive", results[0]["lastActive"]}
        };
        std::string token = CustomSerial::encodeJWT(userData.dump());

        // salvăm token în ServerMng
        ServerMng::getInstance()->addToken(token);

        // răspuns către client
        json response = {
            {"controller", "Auth"},
            {"action", "login"},
            {"status", "success"},
            {"token", token},
            {"username", username},
            {"role", results[0]["role"]}
        };
        client->send(response.dump());
        logger->log("User logged in: " + username);
    }
    else
    {
        json response = {
            {"controller", "Auth"},
            {"action", "login"},
            {"status", "error"},
            {"message", "Invalid username or password"}
        };
        client->send(response.dump());
        logger->log("Login failed for user: " + username);
    }
}

void UserController::Register(const json& data, Connection* client)
{
    try {
        std::string username = data["payload"].value("username", "");
        std::string password = data["payload"].value("password", "");
        std::string email = data["payload"].value("email", "");
        std::string role = "common"; // implicit

        if (username.empty() || password.empty() || email.empty()) 
        {
            client->send("ERROR: Missing fields");
            return;
        }

        // Verifică dacă există deja
        std::string checkQuery = "SELECT * FROM Users WHERE Username = '" + username + "'";
        auto existing = dbController->executeQuery(checkQuery);
        if (!existing.empty()) 
        {
            json response = {
                {"controller", "Auth"},
                {"action", "register"},
                {"status", "error"},
                {"message", "Username already exists"}
            };
            client->send(response.dump());
            return;
        }

        // Generează ID (poți folosi și UUID)
        std::string userId = std::to_string(rand()); // temporar, înlocuiește cu un generator sigur

        std::string insertQuery = "INSERT INTO Users (UserID, Username, PasswordHash, Email, Role) VALUES ('" +
            userId + "', '" + username + "', '" + password + "', '" + email + "', '" + role + "')";

        if (dbController->executeUpdate(insertQuery)) {
            json payload = {
                {"userId", userId},
                {"username", username},
                {"role", role}
            };

            std::string token = CustomSerial::encodeJWT(payload.dump());
            ServerMng::getInstance()->addToken(token);

            json response = {
                {"controller", "Auth"},
                {"action", "register"},
                {"status", "success"},
                {"token", token},
                {"username", username}
            };
            client->send(response.dump());
            logger->log("User registered: " + username);
        }
        else {
            client->send("ERROR: Failed to insert user");
        }
    }
    catch (const std::exception& e) {
        client->send("ERROR: Register failed");
        logger->log(std::string("[Register] Exception: ") + e.what());
    }
}

void UserController::Update(const json& data, Connection* client) // to do
{
}

void UserController::Delete(const json& data, Connection* client) // to do
{
}

void UserController::Logout(const json& data, Connection* client) 
{
    try {
        std::string token = data.value("token", "");

        if (!token.empty()) {
            ServerMng::getInstance()->removeToken(token);

            json response = {
                {"controller", "Auth"},
                {"action", "logout"},
                {"status", "success"},
                {"message", "Logged out"}
            };
            client->send(response.dump());
            logger->log("User logged out with token: " + token);
        }
        else {
            client->send("ERROR: Missing token");
        }
    }
    catch (const std::exception& e) {
        client->send("ERROR: Logout failed");
        logger->log(std::string("[Logout] Exception: ") + e.what());
    }
}

void UserController::handleRequest(const std::string& data, Connection* client) 
{
    if (!validateRequest(data, client)) 
    {
        return;
    }

    try {
        json j = json::parse(data);

        std::string action = j["action"];
        json payload = j["payload"];

        if (action == "login") 
        {
			Login(j, client);
        }

        else if (action == "register") 
        {
			Register(j, client);
        }

        else if (action == "update") 
        {
            Update(j, client);
        }

        else if (action == "delete") 
        {
            Delete(j, client);
        }

        else if (action == "logout") 
        {
            Logout(j, client);
        }

        else 
        {
            client->send("ERROR: Invalid action");
        }
    }
    catch (const std::exception& e) {
        client->send("ERROR: Request processing failed");
        logger->log(std::string("[UserController] Exception: ") + e.what());
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

void UserController::loadUserByUsername(const std::string& username) {
    for (auto it = users.begin(); it != users.end(); ++it) {
        if (it->second && it->second->GetUsername() == username) {
            return;
        }
    }

    std::string query = "SELECT * FROM Users WHERE username = '" + username + "'";
    auto results = dbController->executeQuery(query);

    if (!results.empty()) {
        std::string userId = results[0]["userId"];
        User* user = UsersFactory::CreateUser(UserType::COMMON, username, userId).release();
        users[userId] = user;
        logger->log("Loaded user by username: " + username);
    }
}

void UserController::loadUserByEmail(const std::string& email) {
    for (auto it = users.begin(); it != users.end(); ++it) 
    {
        if (it->second && it->second->GetEmail() == email) 
        {
            return;
        }
    }

    std::string query = "SELECT * FROM Users WHERE email = '" + email + "'";
    auto results = dbController->executeQuery(query);

    if (!results.empty()) 
    {
        std::string userId = results[0]["userId"];
        std::string username = results[0]["username"];
        User* user = UsersFactory::CreateUser(UserType::COMMON, username, userId).release();
        user->SetEmail(email);
        users[userId] = user;
        logger->log("Loaded user by email: " + email);
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