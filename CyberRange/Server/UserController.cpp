#include "UsersFactory.h"
#include "UserController.h"
#include "ServerMng.h"
#include "CustomSerial.h"
#include <iostream>


UserController::UserController(DBController* dbCtrl)
    : CController("UserController"), dbController(dbCtrl) {
}

void UserController::Login(const json& data, Connection* client)
{
    std::cout << "[Server] Received login request:\n" << data.dump(4) << "\n";
    std::string username = data["payload"]["username"];
    std::string password = data["payload"]["password"];


    int id = ServerMng::getInstance()->getLoader()->loadUserByUsername(username, client);
    if (id == -1) id = ServerMng::getInstance()->getLoader()->loadUserByEmail(username, client);

    if (id != -1)
    {
        auto* user = ServerMng::getInstance()->getUsers()[id].first;
        if (user->GetPassword() == password) {
            json userData =
            {
                {"userId", id},
                {"username", user->GetUsername()},
                {"email", user->GetEmail()},
                {"role", user->GetAccessLevel()}
            };
            std::string token = CustomSerial::encodeJWT(userData.dump(), ServerMng::getInstance()->getSecretKey());

            ServerMng::getInstance()->addToken(token);

            json response = {
                {"controller", "AuthController"},
                {"action", "login"},
                {"status", "success"},
                {"token", token},
                {"username", user->GetUsername()},
                {"role", user->GetAccessLevel()}
            };
            std::cout << "[Server] Sending response:\n" << response.dump(4) << "\n";
            client->send(response.dump());
            logger->log("User logged in: " + username);
            return;
        }
    }

    json response = {
        {"controller", "AuthController"},
        {"action", "login"},
        {"status", "error"},
        {"message", "Invalid username or password"}
    };
    std::cout << "[Server] Sending response:\n" << response.dump(4) << "\n";
    client->send(response.dump());
    logger->log("Login failed for user: " + username);

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
            json response = {
                {"controller", "Auth"},
                {"action", "register"},
                {"status", "error"},
                {"message", "Missing required fields"}
            };
            client->send(response.dump());
            return;
        }

        // Check if user already exists
        std::string checkQuery = "SELECT * FROM Users WHERE Username = '" + username + "' OR Email = '" + email + "'";
        auto existing = dbController->executeQuery(checkQuery);
        if (!existing.empty())
        {
            json response = {
                {"controller", "Auth"},
                {"action", "register"},
                {"status", "error"},
                {"message", "Username or email already exists"}
            };
            client->send(response.dump());
            return;
        }

        // Generate a more reliable unique ID
        int userId = rand() % 1000000 + 1000000; // Temporary solution
        std::string checkIdQuery = "SELECT * FROM Users WHERE UserID = '" + std::to_string(userId) + "'";
        while (!dbController->executeQuery(checkIdQuery).empty()) {
            userId = rand() % 1000000 + 1000000;
            checkIdQuery = "SELECT * FROM Users WHERE UserID = '" + std::to_string(userId) + "'";
        }

        std::string insertQuery = "INSERT INTO Users (UserID, Username, PasswordHash, Email, Role) VALUES ('" +
            std::to_string(userId) + "', '" + username + "', '" + password + "', '" + email + "', '" + role + "')";

        if (dbController->executeUpdate(insertQuery)) {
            json payload = {
                {"userId", userId},
                {"username", username},
                {"email", email},
                {"role", role}
            };

            std::string token = CustomSerial::encodeJWT(payload.dump(), ServerMng::getInstance()->getSecretKey());
            ServerMng::getInstance()->addToken(token);

            json response = {
                {"controller", "Auth"},
                {"action", "register"},
                {"status", "success"},
                {"token", token},
                {"username", username},
                {"role", role}
            };
            client->send(response.dump());
            logger->log("User registered: " + username);

            // Load the user into memory
            ServerMng::getInstance()->getLoader()->loadUser(userId, client);
        }
        else {
            json response = {
                {"controller", "Auth"},
                {"action", "register"},
                {"status", "error"},
                {"message", "Failed to insert user"}
            };
            client->send(response.dump());
        }
    }
    catch (const std::exception& e) {
        json response = {
            {"controller", "Auth"},
            {"action", "register"},
            {"status", "error"},
            {"message", "Registration failed: " + std::string(e.what())}
        };
        client->send(response.dump());
        logger->log(std::string("[Register] Exception: ") + e.what());
    }
}

void UserController::Update(const json& data, Connection* client)
{
    try {
        if (!data.contains("token") || !data.contains("payload")) {
            json response = {
                {"controller", "UserController"},
                {"action", "update"},
                {"status", "error"},
                {"message", "Missing token or payload"}
            };
            client->send(response.dump());
            return;
        }

        std::string token = data["token"];
        auto& tokens = ServerMng::getInstance()->getTokens();
        if (std::find(tokens.begin(), tokens.end(), token) == tokens.end()) {
            json response = {
                {"controller", "UserController"},
                {"action", "update"},
                {"status", "error"},
                {"message", "Invalid or expired token"}
            };
            client->send(response.dump());
            return;
        }

        // Decode token to get user ID
        json userData = CustomSerial::decodeJWT(token, ServerMng::getInstance()->getSecretKey());
        int userId = userData["userId"];

        // Prepare update fields
        std::vector<std::string> updateFields;
        if (data["payload"].contains("username")) {
            updateFields.push_back("Username = '" + data["payload"]["username"].get<std::string>() + "'");
        }
        if (data["payload"].contains("email")) {
            updateFields.push_back("Email = '" + data["payload"]["email"].get<std::string>() + "'");
        }
        if (data["payload"].contains("password")) {
            updateFields.push_back("PasswordHash = '" + data["payload"]["password"].get<std::string>() + "'");
        }

        if (updateFields.empty()) {
            json response = {
                {"controller", "UserController"},
                {"action", "update"},
                {"status", "error"},
                {"message", "No fields to update"}
            };
            client->send(response.dump());
            return;
        }

        // Build and execute update query
        std::string updateQuery = "UPDATE Users SET ";
        for (size_t i = 0; i < updateFields.size(); ++i) {
            updateQuery += updateFields[i];
            if (i < updateFields.size() - 1) {
                updateQuery += ", ";
            }
        }
        updateQuery += " WHERE UserID = '" + std::to_string(userId) + "'";

        if (dbController->executeUpdate(updateQuery)) {
            // Update user in memory
            auto& users = ServerMng::getInstance()->getUsers();
            if (users.find(userId) != users.end() && users[userId].first != nullptr) {
                User* user = users[userId].first;
                if (data["payload"].contains("username")) {
                    user->SetUsername(data["payload"]["username"]);
                }
                if (data["payload"].contains("email")) {
                    user->SetEmail(data["payload"]["email"]);
                }
                if (data["payload"].contains("password")) {
                    user->SetPassword(data["payload"]["password"]);
                }

                // Create new token with updated info
                json newUserData = {
                    {"userId", userId},
                    {"username", user->GetUsername()},
                    {"email", user->GetEmail()},
                    {"role", user->GetAccessLevel()}
                };
                std::string newToken = CustomSerial::encodeJWT(newUserData.dump(), ServerMng::getInstance()->getSecretKey());

                // Invalidate old token and add new one
                ServerMng::getInstance()->removeToken(token);
                ServerMng::getInstance()->addToken(newToken);

                json response = {
                    {"controller", "UserController"},
                    {"action", "update"},
                    {"status", "success"},
                    {"token", newToken},
                    {"message", "User updated successfully"}
                };
                client->send(response.dump());
                logger->log("User updated: " + std::to_string(userId));
            }
            else {
                json response = {
                    {"controller", "UserController"},
                    {"action", "update"},
                    {"status", "error"},
                    {"message", "User not found in memory"}
                };
                client->send(response.dump());
            }
        }
        else {
            json response = {
                {"controller", "UserController"},
                {"action", "update"},
                {"status", "error"},
                {"message", "Failed to update user in database"}
            };
            client->send(response.dump());
        }
    }
    catch (const std::exception& e) {
        json response = {
            {"controller", "UserController"},
            {"action", "update"},
            {"status", "error"},
            {"message", "Update failed: " + std::string(e.what())}
        };
        client->send(response.dump());
        logger->log(std::string("[Update] Exception: ") + e.what());
    }
}

void UserController::Delete(const json& data, Connection* client)
{
    try {
        if (!data.contains("token") || !data.contains("payload")) {
            json response = {
                {"controller", "UserController"},
                {"action", "delete"},
                {"status", "error"},
                {"message", "Missing token or payload"}
            };
            client->send(response.dump());
            return;
        }

        std::string token = data["token"];
        auto& tokens = ServerMng::getInstance()->getTokens();
        if (std::find(tokens.begin(),tokens.end(),token)==tokens.end()) {
            json response = {
                {"controller", "UserController"},
                {"action", "delete"},
                {"status", "error"},
                {"message", "Invalid or expired token"}
            };
            client->send(response.dump());
            return;
        }

        // Decode token to get user ID
        json userData = CustomSerial::decodeJWT(token, ServerMng::getInstance()->getSecretKey());
        int userId = userData["userId"];

        // Check if user exists in database
        std::string checkQuery = "SELECT * FROM Users WHERE UserID = '" + std::to_string(userId) + "'";
        auto results = dbController->executeQuery(checkQuery);
        if (results.empty()) {
            json response = {
                {"controller", "UserController"},
                {"action", "delete"},
                {"status", "error"},
                {"message", "User not found"}
            };
            client->send(response.dump());
            return;
        }

        // Remove user from teams
        std::string teamQuery = "DELETE FROM UserTeams WHERE UserID = '" + std::to_string(userId) + "'";
        dbController->executeUpdate(teamQuery);

        // Delete user from database
        std::string deleteQuery = "DELETE FROM Users WHERE UserID = '" + std::to_string(userId) + "'";
        if (dbController->executeUpdate(deleteQuery)) {
            // Remove user from memory and invalidate token
            auto& users = ServerMng::getInstance()->getUsers();
            if (users.find(userId) != users.end()) {
                delete users[userId].first;
                users.erase(userId);
            }

            ServerMng::getInstance()->removeToken(token);

            json response = {
                {"controller", "UserController"},
                {"action", "delete"},
                {"status", "success"},
                {"message", "User deleted successfully"}
            };
            client->send(response.dump());
            logger->log("User deleted: " + std::to_string(userId));
        }
        else {
            json response = {
                {"controller", "UserController"},
                {"action", "delete"},
                {"status", "error"},
                {"message", "Failed to delete user from database"}
            };
            client->send(response.dump());
        }
    }
    catch (const std::exception& e) {
        json response = {
            {"controller", "UserController"},
            {"action", "delete"},
            {"status", "error"},
            {"message", "Delete failed: " + std::string(e.what())}
        };
        client->send(response.dump());
        logger->log(std::string("[Delete] Exception: ") + e.what());
    }
}

void UserController::Logout(const json& data, Connection* client)
{
    try {
        std::string token = data.value("token", "");

        if (!token.empty()) {
            ServerMng::getInstance()->removeToken(token);

            json response = {
                {"controller", "AuthController"},
                {"action", "logout"},
                {"status", "success"},
                {"message", "Logged out"}
            };
            client->send(response.dump());
            logger->log("User logged out with token: " + token);
        }
        else {
            json response = {
                {"controller", "AuthController"},
                {"action", "logout"},
                {"status", "error"},
                {"message", "Missing token"}
            };
            client->send(response.dump());
        }
    }
    catch (const std::exception& e) {
        json response = {
            {"controller", "AuthController"},
            {"action", "logout"},
            {"status", "error"},
            {"message", "Logout failed: " + std::string(e.what())}
        };
        client->send(response.dump());
        logger->log(std::string("[Logout] Exception: ") + e.what());
    }
}

void UserController::handleRequest(const std::string& data, Connection* client)
{
    try {
        json j = json::parse(data);

        std::string action = j["action"];

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
            if (!validateRequest(data, client, 1)) return;
            Update(j, client);
        }
        else if (action == "delete")
        {
            if (!validateRequest(data, client, 1)) return;
            Delete(j, client);
        }
        else if (action == "logout")
        {
            if (!validateRequest(data, client, 1)) return;
            Logout(j, client);
        }
        else
        {
            json response = {
                {"controller", "UserController"},
                {"action", action},
                {"status", "error"},
                {"message", "Invalid action"}
            };
            client->send(response.dump());
        }
    }
    catch (const std::exception& e) {
        json response = {
            {"controller", "UserController"},
            {"status", "error"},
            {"message", "Request processing failed: " + std::string(e.what())}
        };
        client->send(response.dump());
        logger->log(std::string("[UserController] Exception: ") + e.what());
    }
}