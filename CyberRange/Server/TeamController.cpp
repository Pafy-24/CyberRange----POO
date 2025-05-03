#include "TeamController.h"
#include "UsersFactory.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

TeamController::TeamController(DBController* dbCtrl)
    : CController("TeamController"), dbController(dbCtrl) {
}

void TeamController::handleRequest(const std::string& data, Connection* client) {
    if (!validateRequest(data, client)) {
        return;
    }

    try {
        json j = json::parse(data);
        std::string action = j["action"].get<std::string>();

        if (action == "createTeam") {
            std::string teamName = j["teamName"].get<std::string>();

            std::string query = "INSERT INTO Teams (teamName) VALUES ('" +
                 teamName + "')";
            if (dbController->executeUpdate(query)) {
				query = "SELECT teamId FROM Teams WHERE teamName = '" + teamName + "'";
				std::string teamId = dbController->executeQuery(query)[0]["teamId"];
                
                client->send(json{ {"status", "success"}, {"teamId", std::stoi(teamId)}}.dump());
                logger->log("Team created: " + teamId);
            }
            else {
                client->send("ERROR: Failed to create team");
            }
        }
        else if (action == "getTeam") {
            std::string teamId = j["teamId"].get<std::string>();
            loadTeam(teamId);

            if (teams.find(teamId) != teams.end()) {
                json response = {
                    {"status", "success"},
                    {"teamId", teamId},
                    {"teamName", teams[teamId]->GetName()}
                };
                client->send(response.dump());
            }
            else {
                client->send("ERROR: Team not found");
            }
        }
        else {
            client->send("ERROR: Invalid action");
        }
    }
    catch (const std::exception& e) {
        client->send("ERROR: Request processing failed");
        logger->log("TeamController error: " + std::string(e.what()));
    }
}

void TeamController::loadTeam(const std::string& teamId) {
    if (teams.find(teamId) != teams.end()) {
        return;
    }

    std::string query = "SELECT * FROM Teams WHERE teamId = '" + teamId + "'";
    auto results = dbController->executeQuery(query);

    if (!results.empty()) {
		Team* team = UsersFactory::CreateTeam(results[0]["teamName"], 0, std::stoi(results[0]["teamId"])).release();
        teams[teamId] = team;
        logger->log("Loaded team: " + teamId);
    }
}

void TeamController::unloadTeam(const std::string& teamId) {
    auto it = teams.find(teamId);
    if (it != teams.end()) {
        std::string query = "UPDATE Teams SET lastActive = GETDATE() WHERE teamId = '" + teamId + "'";
        dbController->executeUpdate(query);
        delete it->second;
        teams.erase(it);
        logger->log("Unloaded team: " + teamId);
    }
}