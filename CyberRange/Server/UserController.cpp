#include "UsersFactory.h"
#include "UserController.h"
#include "ServerMng.h"
#include "CustomSerial.h"
#include <iostream>


UserController::UserController()
    : CController("UserController") {
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



// Modificare pentru funcția Register din UserController.cpp

void UserController::Register(const json& data, Connection* client)
{
    try {
        std::string username = data["payload"].value("username", "");
        std::string password = data["payload"].value("password", "");
        std::string email = data["payload"].value("email", "");
        std::string role = "student"; // implicit

        if (username.empty() || password.empty() || email.empty())
        {
            json response = {
                {"controller", "AuthController"},
                {"action", "register"},
                {"status", "error"},
                {"message", "Missing required fields"}
            };
            client->send(response.dump());
            return;
        }

        // Verificăm dacă utilizatorul există deja folosind loader
        int existingUserId = ServerMng::getInstance()->getLoader()->loadUserByUsername(username, client);
        if (existingUserId != -1) {
            json response = {
                {"controller", "AuthController"},
                {"action", "register"},
                {"status", "error"},
                {"message", "Username already exists"}
            };
            client->send(response.dump());
            return;
        }

        // Verificăm și adresa de email
        existingUserId = ServerMng::getInstance()->getLoader()->loadUserByEmail(email, client);
        if (existingUserId != -1) {
            json response = {
                {"controller", "AuthController"},
                {"action", "register"},
                {"status", "error"},
                {"message", "Email already exists"}
            };
            client->send(response.dump());
            return;
        }

        // Creăm obiectul User nou
        auto newUserPtr = UsersFactory::CreateUser(username, email, -1); // ID va fi generat la salvare
        User* newUser = newUserPtr.release();
        newUser->SetPassword(password);
        int r;
		if (role == "admin") r = 10;
		else if (role == "writer") r = 5;
		else r = 1;
        newUser->SetAccessLevel(r);


		auto users = ServerMng::getInstance()->getUsers();
        ServerMng::getInstance()->pushUser(newUser, client);
        ServerMng::getInstance()->getLoader()->registerObject(client, "user:" + std::to_string(-1));
        ServerMng::getInstance()->getLoader()->save(client);
        ServerMng::getInstance()->getLoader()->unloadObject(client, "user:" + std::to_string(-1));

        

        json response = {
            {"controller", "AuthController"},
            {"action", "register"},
            {"status", "success"}
        };
        client->send(response.dump());
        logger->log("User registered: " + username);
    }
    catch (const std::exception& e) {
        json response = {
            {"controller", "AuthController"},
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
                {"controller", "AuthController"},
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
                {"controller", "AuthController"},
                {"action", "update"},
                {"status", "error"},
                {"message", "Invalid or expired token"}
            };
            client->send(response.dump());
            return;
        }

        // Decodăm token-ul pentru a obține ID-ul utilizatorului
        std::string userDataStr = CustomSerial::decodeJWT(token, ServerMng::getInstance()->getSecretKey());
        json userData = json::parse(userDataStr);
        int userId = userData["userId"];

        // Ne asigurăm că utilizatorul este încărcat
        ServerMng::getInstance()->getLoader()->loadUser(userId, client);

        auto& users = ServerMng::getInstance()->getUsers();
        if (users.find(userId) != users.end() && users[userId].first != nullptr) {
            User* user = users[userId].first;
            bool needsUpdate = false;

            // Verificăm și actualizăm numele de utilizator
            if (data["payload"].contains("username") &&
                data["payload"]["username"].is_string() &&
                data["payload"]["username"].get<std::string>().size() > 3) {

                std::string newUsername = data["payload"]["username"];
                // Verificăm dacă numele este deja utilizat de altcineva
                int existingId = ServerMng::getInstance()->getLoader()->loadUserByUsername(newUsername, client);
                if (existingId != -1 && existingId != userId) {
                    json response = {
                        {"controller", "AuthController"},
                        {"action", "update"},
                        {"status", "error"},
                        {"message", "Username already exists"}
                    };
                    client->send(response.dump());
                    return;
                }

                user->SetUsername(newUsername);
                needsUpdate = true;
            }

            // Verificăm și actualizăm email-ul
            if (data["payload"].contains("email") &&
                data["payload"]["email"].is_string() &&
                data["payload"]["email"].get<std::string>().size() > 3 &&
                data["payload"]["email"].get<std::string>().find('@') != std::string::npos) {

                std::string newEmail = data["payload"]["email"];
                // Verificăm dacă email-ul este deja utilizat de altcineva
                int existingId = ServerMng::getInstance()->getLoader()->loadUserByEmail(newEmail, client);
                if (existingId != -1 && existingId != userId) {
                    json response = {
                        {"controller", "AuthController"},
                        {"action", "update"},
                        {"status", "error"},
                        {"message", "Email already exists"}
                    };
                    client->send(response.dump());
                    return;
                }

                user->SetEmail(newEmail);
                needsUpdate = true;
            }

            if (data["payload"].contains("oldPassword") &&
                data["payload"].contains("password")) {
                if (data["payload"]["oldPassword"] == user->GetPassword())
                    if (data["payload"]["password"].get<std::string>().size() > 3) {
                        user->SetPassword(data["payload"]["password"]);
                        needsUpdate = true;
                    }
            }

            if (needsUpdate) {
                ServerMng::getInstance()->getLoader()->save(client);
                std::string objId = "user:" + std::to_string(userId);
                ServerMng::getInstance()->getLoader()->unloadObject(client, objId);
                ServerMng::getInstance()->getLoader()->loadUser(userId,client);

				user = ServerMng::getInstance()->getUsers()[userId].first;
                json newUserData = {
                    {"userId", userId},
                    {"username", user->GetUsername()},
                    {"email", user->GetEmail()},
                    {"role", user->GetAccessLevel()}
                };
                std::string newToken = CustomSerial::encodeJWT(newUserData.dump(), ServerMng::getInstance()->getSecretKey());

                // Actualizăm token-ul
                ServerMng::getInstance()->removeToken(token);
                ServerMng::getInstance()->addToken(newToken);

                json response = {
                    {"controller", "AuthController"},
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
                    {"controller", "AuthController"},
                    {"action", "update"},
                    {"status", "error"},
                    {"message", "No fields to update or invalid data provided"}
                };
                client->send(response.dump());
            }
        }
        else {
            json response = {
                {"controller", "AuthController"},
                {"action", "update"},
                {"status", "error"},
                {"message", "User not found in memory"}
            };
            client->send(response.dump());
        }
    }
    catch (const std::exception& e) {
        json response = {
            {"controller", "AuthController"},
            {"action", "update"},
            {"status", "error"},
            {"message", "Update failed: " + std::string(e.what())}
        };
        client->send(response.dump());
        logger->log(std::string("[Update] Exception: ") + e.what());
    }
}


void UserController::Logout(const json& data, Connection* client)
{
    try {
        std::string token = data.value("token", "");

        if (!token.empty()) {

            ServerMng::getInstance()->getLoader()->saveUnload(client);
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
        auto results = ServerMng::getInstance()->getDBController()->executeQuery(checkQuery);
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
        ServerMng::getInstance()->getDBController()->executeUpdate(teamQuery);

        // Delete user from database
        std::string deleteQuery = "DELETE FROM Users WHERE UserID = '" + std::to_string(userId) + "'";
        if (ServerMng::getInstance()->getDBController()->executeUpdate(deleteQuery)) {
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