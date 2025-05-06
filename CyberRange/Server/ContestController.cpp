#include "ContestController.h"
#include "ServerMng.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

ContestController::ContestController(DBController* dbCtrl) 
    : CController("ContestController"), dbController(dbCtrl) {}

void ContestController::createContest(const std::string& contestData, Connection* conn)
{
    try {
        json j = json::parse(contestData);
        std::string name = j["name"].get<std::string>();

        Contest* contest = new Contest(name);

        ServerMng::getInstance()->getChallMng()->addContest(contest);

        ServerMng::getInstance()->getLoader()->save(conn);

        // Unload and reload the contest
        
        ServerMng::getInstance()->getLoader()->unloadObject(conn,"contest:"+std::to_string(contest->getId()));

        logger->log("Contest created and managed: " + name);
    }
    catch (const std::exception& e) {
        logger->log("createContest error: " + std::string(e.what()));
    }
}

void ContestController::handleRequest(const std::string& data, Connection* client)
{
    if (!validateRequest(data, client, 1)) {
        return;
    }

    try {
        json j = json::parse(data);
        std::string action = j["action"].get<std::string>();

        if (action == "createContest") {
            createContest(j["payload"].dump(),client);
            json response = {
                {"controller", "ContestController"},
                {"action", "createContest"},
                {"status", "success"},
                {"message", "Contest created successfully"}
            };
            client->send(response.dump());
        } else if (action == "getContest") {
            int contestId = j["payload"]["contestId"].get<int>();
            sendContestDetails(contestId, client);
        } else if (action == "getScoreboard") {
            int contestId = j["payload"]["contestId"].get<int>();
            sendScoreboard(contestId, client);
        } else if (action == "getContests") {
            sendAllContests(client);
        } else if (action == "updateContest") {
            int contestId = j["payload"]["contestId"].get<int>();
           // updateContest(std::to_string(contestId), j["payload"].dump());
            json response = {
                {"controller", "ContestController"},
                {"action", "updateContest"},
                {"status", "success"},
                {"message", "Contest updated successfully"}
            };
            client->send(response.dump());
        } else if (action == "deleteContest") {
            int contestId = j["payload"]["contestId"].get<int>();
           // deleteContest(std::to_string(contestId));
            json response = {
                {"controller", "ContestController"},
                {"action", "deleteContest"},
                {"status", "success"},
                {"message", "Contest deleted successfully"}
            };
            client->send(response.dump());
        } else {
            json response = {
                {"controller", "ContestController"},
                {"action", action},
                {"status", "error"},
                {"message", "Invalid action"}
            };
            client->send(response.dump());
        }
    } catch (const std::exception& e) {
        json response = {
            {"controller", "ContestController"},
            {"action", "unknown"},
            {"status", "error"},
            {"message", "Request processing failed: " + std::string(e.what())}
        };
        client->send(response.dump());
        logger->log("ContestController error: " + std::string(e.what()));
    }
}

void ContestController::sendContestDetails(int contestId, Connection* client)
{
    ServerMng::getInstance()->getLoader()->loadContest(contestId, client);
    Contest* contest = ServerMng::getInstance()->getContest(contestId);

    if (contest) {
        json response = {
            {"controller", "ContestController"},
            {"action", "getContest"},
            {"status", "success"},
            {"contestId", contestId},
            {"name", contest->getName()},
            {"description", contest->getDescription()},
            {"startTime", contest->getStartTime()},
            {"endTime", contest->getEndTime()},
            {"active", contest->isActive()}
        };
        client->send(response.dump());
    } else {
        json response = {
            {"controller", "ContestController"},
            {"action", "getContest"},
            {"status", "error"},
            {"message", "Contest not found"}
        };
        client->send(response.dump());
    }
}

void ContestController::sendScoreboard(int contestId, Connection* client)
{
    Scoreboard* sb;// = getScoreboard(std::to_string(contestId));
    /*if (sb) {
        std::string scoreboardJson = sb->exportJSON();
        delete sb;
        json response = {
            {"controller", "ContestController"},
            {"action", "getScoreboard"},
            {"status", "success"},
            {"scoreboard", json::parse(scoreboardJson)}
        };
        client->send(response.dump());
    } else {
        json response = {
            {"controller", "ContestController"},
            {"action", "getScoreboard"},
            {"status", "error"},
            {"message", "Scoreboard not found"}
        };
        client->send(response.dump());
    }*/
}

void ContestController::sendAllContests(Connection* client)
{
    std::string query = "SELECT ContestID FROM Contests";
    auto results = dbController->executeQuery(query);
    std::vector<json> contestList;

    for (const auto& row : results) {
        int contestId = std::stoi(row.at("ContestID"));
        ServerMng::getInstance()->getLoader()->loadContest(contestId, client);
        Contest* contest = ServerMng::getInstance()->getContest(contestId);
        if (contest) {
            contestList.push_back({
                {"contestId", contestId},
                {"name", contest->getName()}
            });
        }
    }

    json response = {
        {"controller", "ContestController"},
        {"action", "getContests"},
        {"status", "success"},
        {"contests", contestList}
    };
    client->send(response.dump());
}
