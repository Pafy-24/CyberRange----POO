#include "ChallController.h"
#include "json.hpp"
#include "ServerMng.h"
#include <iostream>

using json = nlohmann::json;

ChallController::ChallController()
    : CController("ChallController") {
}

void ChallController::handleRequest(const std::string& data, Connection* client) {
    if (!validateRequest(data, client, 1)) {
        return;
    }

    auto dbController = ServerMng::getInstance()->getDBController();

    try {
        json j = json::parse(data);
        std::string action = j["action"].get<std::string>();

        if (action == "getChallengeList") {
            std::string query = "SELECT * FROM Challenges";
            auto results = dbController->executeQuery(query);

            if (!results.empty()) {
                json data = json::array();
                for (const auto& row : results) {
                    data.push_back({
                        {"challId", row.at("ChallengeID")},
                        {"title", row.at("Name")},
                        {"difficulty", row.at("Difficulty")},
                        {"score", row.at("Score")},
                        {"tags", row.at("Tags")},
                        {"tabId", row.at("TabID")},
                        {"contestId", row.at("ContestID")}
                        });
                }

                client->send(json{
                    {"status", "success"},
                    {"action", "getChallengeList"},
                    {"data", data}
                    }.dump());
                logger->log("Challenge list retrieved successfully.");
            }
            else {
                client->send(json{ {"status", "error"}, {"message", "No challenges found"} }.dump());
            }

        }
        else if (action == "getChallengeDetails") {
            std::string challId = j["challId"].get<std::string>();
            loadChallenge(challId);

            if (challenges.find(challId) != challenges.end()) {
                json response = {
                    {"status", "success"},
                    {"action", "getChallengeDetails"},
                    {"data", {
                        {"challId", challId},
                        {"title", challenges[challId]->getName()},
                        {"description", challenges[challId]->getDescription()},
                        {"flag", challenges[challId]->getFlag()}
                    }}
                };
                client->send(response.dump());
                logger->log("Retrieved challenge details: " + challId);
            }
            else {
                client->send(json{ {"status", "error"}, {"message", "Challenge not found"} }.dump());
                logger->log("Challenge not found: " + challId);
            }

        }
        else if (action == "submitFlag") {
            std::string challId = j["challId"].get<std::string>();
            std::string flag = j["flag"].get<std::string>();
            std::string userId = j["userId"].get<std::string>();

            loadChallenge(challId);
            if (challenges.find(challId) != challenges.end()) {
                if (challenges[challId]->validateFlag(flag)) {
                    std::string query = "INSERT INTO SolvedChallenges (userId, challId) VALUES (?, ?)";
                    if (dbController->executeUpdate(query, { userId, challId })) {
                        client->send(json{ {"status", "success"}, {"message", "Flag correct"} }.dump());
                        logger->log("Flag submitted: " + challId + " by user: " + userId);
                    }
                    else {
                        client->send(json{ {"status", "error"}, {"message", "Failed to record solution"} }.dump());
                        logger->log("Failed to record solution for: " + challId);
                    }
                }
                else {
                    client->send(json{ {"status", "error"}, {"message", "Incorrect flag"} }.dump());
                    logger->log("Incorrect flag for: " + challId);
                }
            }
            else {
                client->send(json{ {"status", "error"}, {"message", "Challenge not found"} }.dump());
                logger->log("Challenge not found: " + challId);
            }

        }
        else {
            client->send(json{ {"status", "error"}, {"message", "Invalid action"} }.dump());
            logger->log("Invalid action in ChallController: " + action);
        }

    }
    catch (const std::exception& e) {
        client->send(json{ {"status", "error"}, {"message", "Request processing failed"} }.dump());
        logger->log("ChallController error: " + std::string(e.what()));
    }
}


void ChallController::loadChallenge(const std::string& challId) {
    if (challenges.find(challId) != challenges.end()) {
        return;
    }

    std::string query = "SELECT * FROM Challenges WHERE ChallengeID = ?";
    auto results = ServerMng::getInstance()->getDBController()->executeQuery(query, {challId});

    if (!results.empty()) 
    {
        Chall* challenge = new Chall(results[0]["title"], {}, std::stoi(results[0]["challId"]));
        challenge->setDescription(results[0]["description"]);
        challenge->setFlag(results[0]["flag"]);
        challenges[challId] = challenge;
        logger->log("Loaded challenge: " + challId);
    }
    else 
    {
        logger->log("Failed to load challenge: " + challId);
    }
}

void ChallController::unloadChallenge(const std::string& challId) {
    auto it = challenges.find(challId);
    if (it != challenges.end()) {
        std::string query = "UPDATE Challenges SET lastActive = GETDATE() WHERE ChallengeID = ?";
        ServerMng::getInstance()->getDBController()->executeUpdate(query, {challId});
        delete it->second;
        challenges.erase(it);
        logger->log("Unloaded challenge: " + challId);
    }
}
