#include "ContestController.h"
#include "ServerMng.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

ContestController::ContestController()
    : CController("ContestController") {
}

void ContestController::createContest(const std::string& contestData, Connection* conn)
{
    try {
        json j = json::parse(contestData);
        std::string name = j["name"].get<std::string>();
        std::string description = j.value("description", "");
        time_t startTime = j.value("startTime", time(nullptr));
        time_t endTime = j.value("endTime", time(nullptr) + 86400); // Default: 1 day later
        int maxTeamUsers = j.value("maxTeamUsers", 5);
        bool active = j.value("active", true);

        Contest* contest = new Contest(name);
        contest->setDescription(description);
        contest->setStartTime(startTime);
        contest->setEndTime(endTime);
        contest->setMaxTeamUsers(maxTeamUsers);
        contest->setActive(active);

        ServerMng::getInstance()->getChallMng()->addContest(contest);
        ServerMng::getInstance()->getLoader()->saveContest(contest->getId());

        ServerMng::getInstance()->getLoader()->registerObject(conn, "contest:" + std::to_string(contest->getId()));

        logger->log("Contest created and saved: " + name);
    }
    catch (const std::exception& e) {
        logger->log("createContest error: " + std::string(e.what()));
    }
}

void ContestController::updateContest(const std::string& contestId, const std::string& contestData, Connection* client)
{
    try {
        int id = std::stoi(contestId);
        json j = json::parse(contestData);

        ServerMng::getInstance()->getLoader()->loadContest(id, client);
        Contest* contest = ServerMng::getInstance()->getContest(id);

        if (contest) {
            if (j.contains("name")) contest->setName(j["name"].get<std::string>());
            if (j.contains("description")) contest->setDescription(j["description"].get<std::string>());
            if (j.contains("startTime")) contest->setStartTime(j["startTime"].get<time_t>());
            if (j.contains("endTime")) contest->setEndTime(j["endTime"].get<time_t>());
            if (j.contains("maxTeamUsers")) contest->setMaxTeamUsers(j["maxTeamUsers"].get<int>());
            if (j.contains("active")) contest->setActive(j["active"].get<bool>());

            ServerMng::getInstance()->getLoader()->saveContest(id);
            logger->log("Contest updated: " + std::to_string(id));
        }
        else {
            logger->log("Contest not found: " + std::to_string(id));
        }
    }
    catch (const std::exception& e) {
        logger->log("updateContest error: " + std::string(e.what()));
    }
}

void ContestController::deleteContest(const std::string& contestId, Connection* client)
{
    try {
        int id = std::stoi(contestId);
        ServerMng::getInstance()->getLoader()->loadContest(id, client);
        Contest* contest = ServerMng::getInstance()->getContest(id);

        if (contest && 0) {
          //  ServerMng::getInstance()->getLoader()->cleanupContest(id, client);
            logger->log("Contest deleted: " + std::to_string(id));
        }
        else {
            logger->log("Contest not found: " + std::to_string(id));
        }
    }
    catch (const std::exception& e) {
        logger->log("deleteContest error: " + std::string(e.what()));
    }
}

void ContestController::sendContestDetails(int contestId, Connection* client)
{
    ServerMng::getInstance()->getLoader()->loadContest(contestId, client);
    Contest* contest = ServerMng::getInstance()->getContest(contestId);

    if (contest) {
        json response = {
            {"controller", "ContestClientController"},
            {"action", "getContest"},
            {"status", "success"},
            {"contestId", contestId},
            {"name", contest->getName()},
            {"description", contest->getDescription()},
            {"startTime", contest->getStartTime()},
            {"endTime", contest->getEndTime()},
            {"maxTeamUsers", contest->getMaxTeamUsers()},
            {"organizerId",contest->getOrganizerId()},
            {"active", contest->isActive()}
        };

        // Include challenges
        json challenges = json::array();
        for (const auto& challPair : contest->getChallenges()) {
            challenges.push_back({
                {"id", challPair.first},
                {"name", challPair.second->getName()},
                {"points", challPair.second->getPoints()}
                });
        }
        response["challenges"] = challenges;

        client->send(response.dump());
    }
    else {
        json response = {
            {"controller", "ContestClientController"},
            {"action", "getContest"},
            {"status", "error"},
            {"message", "Contest not found"}
        };
        client->send(response.dump());
    }
}

void ContestController::sendScoreboard(int contestId, Connection* client)
{
    ServerMng::getInstance()->getLoader()->loadContest(contestId, client);
    Contest* contest = ServerMng::getInstance()->getContest(contestId);

    if (contest) {
        json response = {
            {"controller", "ContestClientController"},
            {"action", "getScoreboard"},
            {"status", "success"},
            {"contestId", contestId},
            {"scoreboard", json::array()}
        };

        // This is a placeholder - implement actual scoreboard logic based on your requirements
        // You might need to query the database for team scores or user submissions
        client->send(response.dump());
    }
    else {
        json response = {
            {"controller", "ContestClientController"},
            {"action", "getScoreboard"},
            {"status", "error"},
            {"message", "Contest not found"}
        };
        client->send(response.dump());
    }
}

void ContestController::sendAllContests(Connection* client)
{
    std::string query = "SELECT ContestID FROM Contests";
    auto results = ServerMng::getInstance()->getDBController()->executeQuery(query);
    json contestList = json::array();

    for (const auto& row : results) {
        int contestId = std::stoi(row.at("ContestID"));
        ServerMng::getInstance()->getLoader()->loadContest(contestId, client);
        Contest* contest = ServerMng::getInstance()->getContest(contestId);
        if (contest) {
            contestList.push_back({
                {"contestId", contestId},
                {"name", contest->getName()},
				{"startTime", contest->getStartTime()},
				{"endTime", contest->getEndTime()}
                });
        }
    }

    json response = {
        {"controller", "ContestClientController"},
        {"action", "getContests"},
        {"status", "success"},
        {"contests", contestList}
    };
    client->send(response.dump());
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
            createContest(j["payload"].dump(), client);
            json response = {
                {"controller", "ContestClientController"},
                {"action", "createContest"},
                {"status", "success"},
                {"message", "Contest created successfully"}
            };
            client->send(response.dump());
        }
        else if (action == "getContest") {
            int contestId = j["payload"]["contestId"].get<int>();
            sendContestDetails(contestId, client);
        }
        else if (action == "getScoreboard") {
            int contestId = j["payload"]["contestId"].get<int>();
            sendScoreboard(contestId, client);
        }
        else if (action == "getContests") {
            sendAllContests(client);
        }
        else if (action == "updateContest") {
            int contestId = j["payload"]["contestId"].get<int>();
			ServerMng::getInstance()->getLoader()->loadContest(contestId, client);
            updateContest(std::to_string(contestId), j["payload"].dump(),client);
            json response = {
                {"controller", "ContestClientController"},
                {"action", "updateContest"},
                {"status", "success"},
                {"message", "Contest updated successfully"}
            };
            client->send(response.dump());
        }
        else if (action == "deleteContest") {
            int contestId = j["payload"]["contestId"].get<int>();
            deleteContest(std::to_string(contestId),client);
            json response = {
                {"controller", "ContestClientController"},
                {"action", "deleteContest"},
                {"status", "success"},
                {"message", "Contest deleted successfully"}
            };
            client->send(response.dump());
        }
        else {
            json response = {
                {"controller", "ContestClientController"},
                {"action", action},
                {"status", "error"},
                {"message", "Invalid action"}
            };
            client->send(response.dump());
        }
    }
    catch (const std::exception& e) {
        json response = {
            {"controller", "ContestClientController"},
            {"action", "unknown"},
            {"status", "error"},
            {"message", "Request processing failed: " + std::string(e.what())}
        };
        client->send(response.dump());
        logger->log("ContestController error: " + std::string(e.what()));
    }
}