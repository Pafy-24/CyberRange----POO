#include "ChallController.h"
#include "json.hpp"
#include "ServerMng.h"
#include <iostream>

using json = nlohmann::json;

ChallController::ChallController() : CController("ChallController") {}

Chall* ChallController::getChallenge(int challId, int tabId, int contestId, Connection* client) {
    ServerMng* server = ServerMng::getInstance();
    Chall* challenge = nullptr;

    if (tabId) {
        server->getLoader()->loadTab(tabId, client);
        Tab* tab = server->getTab(tabId);
        if (tab) {
            server->getLoader()->loadChall(challId, tab, client);
            auto challengesMap = tab->getChallenges();
            auto it = challengesMap.find(challId);
            if (it != challengesMap.end()) {
                challenge = it->second;
            }
        }
    }
    else if (contestId) {
        server->getLoader()->loadContest(contestId, client);
        Contest* contest = server->getContest(contestId);
        if (contest) {
            server->getLoader()->loadChall(challId, contest, client);
            auto challengesMap = contest->getChallenges();
            auto it = challengesMap.find(challId);
            if (it != challengesMap.end()) {
                challenge = it->second;
            }
        }
    }
    return challenge;
}

bool ChallController::updateLeaderboard(int challId, int tabId, int contestId, int userId,
    int teamId, const std::string& flag, int points,
    Connection* client) {
    std::string query;
    if (tabId) {
        query = "INSERT INTO Leaderboard (TabID, ChallengeID, UserID, FlagSubmitted, Points) VALUES (?,?,?,?,?)";
        return ServerMng::getInstance()->getDBController()->executeUpdate(query, {
            std::to_string(tabId), std::to_string(challId),
            std::to_string(userId), flag, std::to_string(points)
            });
    }
    else {
        query = "INSERT INTO Leaderboard (ContestID, ChallengeID, TeamID, UserID, FlagSubmitted, Points) VALUES (?,?,?,?,?,?)";
        return ServerMng::getInstance()->getDBController()->executeUpdate(query, {
            std::to_string(contestId), std::to_string(challId), std::to_string(teamId),
            std::to_string(userId), flag, std::to_string(points)
            });
    }
}

void ChallController::handleRequest(const std::string& data, Connection* client) {
    if (!client || !validateRequest(data, client, 1)) {
        return;
    }

    try {
        json j = json::parse(data);
        if (!j.contains("action")) {
            client->send(json{ {"status", "error"}, {"message", "Missing action"} }.dump());
            return;
        }

        std::string action = j["action"].get<std::string>();
        int tabId = j.contains("tabId") ? j["tabId"].get<int>() : 0;
        int contestId = j.contains("contestId") ? j["contestId"].get<int>() : 0;

        if (action == "getChallengeDetails") {
            if (!j.contains("challId")) {
                client->send(json{ {"status", "error"}, {"message", "Missing challenge ID"} }.dump());
                return;
            }

            int challId = j["challId"].get<int>();
            Chall* challenge = getChallenge(challId, tabId, contestId, client);




            if (challenge) {
				ServerMng::getInstance()->getLoader()->loadUser(challenge->getAuthor(), client);
                
                json response = {
                    {"status", "success"},
                    {"controller","ChallClientController"},
                    {"action", "getChallengeDetails"},
                    {"data", {
                        {"challId", challId},
                        {"title", challenge->getName()},
						{"description", challenge->getDescription()},
						{"author", ServerMng::getInstance()->getUsers()[challenge->getAuthor()].first->GetUsername()},
						{"points", challenge->getPoints()},
						{"tags", challenge->getTags()},
                        {"difficulty", challenge->getDiffStr()}
                    }}
                };
                client->send(response.dump());
                print("Retrieved challenge details: " + std::to_string(challId));
            }
            else {
                client->send(json{ {"status", "error"}, {"message", "Challenge not found"} }.dump());
                print("Challenge not found: " + std::to_string(challId));
            }
        }
        else if (action == "submitFlag") {
            if (!j.contains("challId") || !j.contains("flag") || !j.contains("userId")) {
                client->send(json{ {"status", "error"}, {"message", "Missing required fields"} }.dump());
                return;
            }

            int challId = j["challId"].get<int>();
            std::string flag = j["flag"].get<std::string>();
            int userId = j["userId"].get<int>();
            int teamId = j.contains("teamId") ? j["teamId"].get<int>() : 0;

            Chall* challenge = getChallenge(challId, tabId, contestId, client);

            if (challenge) {
                if (challenge->validateFlag(flag)) {
                    if (updateLeaderboard(challId, tabId, contestId, userId, teamId,
                        flag, challenge->getPoints(), client)) {
                        client->send(json{ {"status", "success"}, {"message", "Flag correct"} }.dump());
                        print("Flag submitted: " + std::to_string(challId) + " by user: " + std::to_string(userId));
                    }
                    else {
                        client->send(json{ {"status", "error"}, {"message", "Failed to record solution"} }.dump());
                        print("Failed to record solution for: " + std::to_string(challId));
                    }
                }
                else {
                    client->send(json{ {"status", "error"}, {"message", "Incorrect flag"} }.dump());
                    print("Incorrect flag for: " + std::to_string(challId));
                }
            }
            else {
                client->send(json{ {"status", "error"}, {"message", "Challenge not found"} }.dump());
                print("Challenge not found: " + std::to_string(challId));
            }
        }
        else {
            client->send(json{ {"status", "error"}, {"message", "Invalid action"} }.dump());
            print("Invalid action in ChallController: " + action);
        }
    }
    catch (const json::exception& e) {
        client->send(json{ {"status", "error"}, {"message", "Invalid JSON format"} }.dump());
        print("JSON parsing error: " + std::string(e.what()));
    }
    catch (const std::exception& e) {
        client->send(json{ {"status", "error"}, {"message", "Request processing failed"} }.dump());
        print("ChallController error: " + std::string(e.what()));
    }
}

