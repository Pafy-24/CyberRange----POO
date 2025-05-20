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

    try {
        json j = json::parse(data);
        std::string action = j["action"].get<std::string>();

        int tabId=0, contestId=0;
		if (j.contains("tabId")) { tabId = j["tabId"].get<int>(); }
		if (j.contains("contestId")) { contestId = j["contestId"].get<int>(); }

        if (action == "getChallengeDetails") {
            int challId = j["challId"].get<int>();

            Tab* tab = nullptr;
            Contest* contest = nullptr;
            Chall* challenge = nullptr;
            if (tabId) {
                tab = ServerMng::getInstance()->getTab(tabId);
                ServerMng::getInstance()->getLoader()->loadChall(challId, tab, client);
                challenge = tab->getChallenges()[challId];
            }
            else {
                contest = ServerMng::getInstance()->getContest(contestId);
                ServerMng::getInstance()->getLoader()->loadChall(challId, contest, client);
                challenge = contest->getChallenges()[challId];
            }


            if (challenge) {
                json response = {
                    {"status", "success"},
                    {"action", "getChallengeDetails"},
                    {"data", {
                        {"challId", challId},
                        {"title", challenge->getName()},
                        {"description", challenge->getDescription()},
                        {"flag", challenge->getFlag()}
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
            int challId = j["challId"].get<int>();
            std::string flag = j["flag"].get<std::string>();
            int userId = j["userId"].get<int>();

            Tab* tab = nullptr;
            Contest* contest = nullptr;
			Chall* challenge = nullptr;
            if (tabId) {
                tab = ServerMng::getInstance()->getTab(tabId);
                ServerMng::getInstance()->getLoader()->loadChall(challId, tab, client);
                challenge = tab->getChallenges()[challId];
            }
            else {
                contest = ServerMng::getInstance()->getContest(contestId);
                ServerMng::getInstance()->getLoader()->loadChall(challId, contest, client);
                challenge = contest->getChallenges()[challId];
            }


            if (challenge) {
                if (challenge->validateFlag(flag)) {
                    std::string query;
                    if (tabId) {
                        query = "INSERT INTO Leaderboard (TabID, ChallengeID, UserID, FlagSubmitted, Points)  VALUES (?,?,?,?,?)";

                        if (ServerMng::getInstance()->getDBController()->executeUpdate(query, { 
                            std::to_string(tabId), std::to_string(challId), 
                            std::to_string(userId),flag,std::to_string(challenge->getPoints())})) {
                            client->send(json{ {"status", "success"}, {"message", "Flag correct"} }.dump());
                            print("Flag submitted: " + std::to_string(challId) + " by user: " + std::to_string(userId));
                        }
                        else {
                            client->send(json{ {"status", "error"}, {"message", "Failed to record solution"} }.dump());
                            print("Failed to record solution for: " + std::to_string(challId));
                        }
                    }
                    else {
						auto teamId = j["teamId"].get<int>();
                        query = "INSERT INTO Leaderboard (ContestID, ChallengeID, TeamID,UserID, FlagSubmitted, Points)  VALUES (?,?,?,?,?,?)";

                        if (ServerMng::getInstance()->getDBController()->executeUpdate(query, {
                            std::to_string(contestId), std::to_string(challId), std::to_string(teamId),
                            std::to_string(userId),flag,std::to_string(challenge->getPoints()) })) {
                            client->send(json{ {"status", "success"}, {"message", "Flag correct"} }.dump());
                            print("Flag submitted: " + std::to_string(challId) + " by user: " + std::to_string(userId));
                        }
                        else {
                            client->send(json{ {"status", "error"}, {"message", "Failed to record solution"} }.dump());
                            print("Failed to record solution for: " + std::to_string(challId));
                        }

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
    catch (const std::exception& e) {
        client->send(json{ {"status", "error"}, {"message", "Request processing failed"} }.dump());
        print("ChallController error: " + std::string(e.what()));
    }
}
