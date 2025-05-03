#include "CController.h"
#include "json.hpp"
#include "ServerMng.h"
#include <mutex>
#include <algorithm>
#include "CLogger.h"
using json = nlohmann::json;

CController::CController(std::string name) : controllerName(name), logger(new CLogger()) {}

bool CController::validateRequest(const std::string& data, Connection* client) {
    try {
        json j = json::parse(data);
        if (!j.contains("action") || !j.contains("token")) {
            client->send(json{ {"status", "error"}, {"message", "Invalid request format"} }.dump());
            logger->log("Invalid request format");
            return false;
        }

        std::string token = j["token"].get<std::string>();
        ServerMng* server = ServerMng::getInstance();

        auto tokens = server->getTokens();
        if (std::find(tokens.begin(), tokens.end(), token) == tokens.end()) {
            client->send(json{ {"status", "error"}, {"message", "Not logged in"} }.dump());
            logger->log("Invalid token: " + token);
            return false;
        }

        logger->log("Request validated for action: " + j["action"].get<std::string>());
        return true;
    }
    catch (const std::exception& e) {
        client->send(json{ {"status", "error"}, {"message", "Invalid JSON format"} }.dump());
        logger->log("Request validation failed: " + std::string(e.what()));
        return false;
    }
}

void CController::handleRequest(const std::string& data, Connection* client) {
    if (!validateRequest(data, client)) {
        return;
    }
    client->send(json{ {"status", "error"}, {"message", "Not allowed"} }.dump());
    logger->log("Request not allowed for controller: " + controllerName);
}