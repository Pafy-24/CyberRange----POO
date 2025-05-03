#include "ContestController.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

ContestController::ContestController(DBController* dbCtrl) : CController("ContestController"), dbController(dbCtrl) {}

void ContestController::loadContest(const std::string& contestId) 
{
    if (contests.find(contestId) != contests.end())
        return;

    DBController* db = getDB();
    auto results = db->executeQuery("SELECT * FROM Contests WHERE ContestID = ?", { contestId });

    if (!results.empty()) 
    {
        auto& row = results[0];
        Contest* contest = new Contest(row["name"], std::stoi(row["contestId"]));
        contests[contestId] = contest;
        logger->log("Loaded contest: " + contestId);
    }
    else 
    {
        logger->log("Contest not found: " + contestId);
    }
}

void ContestController::createContest(std::string contestData) 
{
    logger->log("createContest not yet implemented");
}

void ContestController::handleRequest(const std::string& data, Connection* client) {
    if (!validateRequest(data, client)) 
    {
        return;
    }

    try {
        json j = json::parse(data);
        std::string action = j["action"].get<std::string>();

        if (action == "createContest") 
        {
            std::string name = j["name"].get<std::string>();

            std::string query = "INSERT INTO Contests (name) VALUES (?)";
            if (getDB()->executeUpdate(query, { name })) 
            {
                query = "SELECT ContestID FROM Contests WHERE Name = ?";
                auto results = getDB()->executeQuery(query, { name });

                if (!results.empty()) 
                {
                    std::string contestId = results[0]["contestId"];
                    client->send(json{ {"status", "success"}, {"contestId", contestId} }.dump());
                    logger->log("Contest created: " + contestId);
                }
                else 
                {
                    client->send(json{ {"status", "error"}, {"message", "Contest creation failed"} }.dump());
                    logger->log("Contest creation failed: no ID returned");
                }
            }
            else 
            {
                client->send(json{ {"status", "error"}, {"message", "Failed to insert contest"} }.dump());
            }
        }
        else if (action == "getContest") 
        {
            std::string contestId = j["contestId"].get<std::string>();
            loadContest(contestId);

            Contest* contest = getContest(contestId);
            if (contest) 
            {
                json response = 
                {
                    {"status", "success"},
                    {"contestId", contestId},
                    {"name", contest->getName()}
                };
                client->send(response.dump());
            }
            else 
            {
                client->send(json{ {"status", "error"}, {"message", "Contest not found"} }.dump());
            }
        }
        else if (action == "getScoreboard") 
        {
            std::string contestId = j["contestId"].get<std::string>();
            Scoreboard* sb = getScoreboard(contestId);

            if (sb) 
            {
                client->send(sb->exportJSON());
                delete sb;
                logger->log("Scoreboard sent for contest: " + contestId);
            }
            else 
            {
                client->send(json{ {"status", "error"}, {"message", "Scoreboard not found"} }.dump());
                logger->log("Failed to generate scoreboard for contest: " + contestId);
            }
        }
        else 
        {
            client->send(json{ {"status", "error"}, {"message", "Invalid action"} }.dump());
        }
    }
    catch (const std::exception& e) 
    {
        client->send(json{ {"status", "error"}, {"message", "Request processing failed"} }.dump());
        logger->log("ContestController error: " + std::string(e.what()));
    }
}

void ContestController::updateContest(std::string contestId, std::string contestData) 
{
    logger->log("updateContest not yet implemented");
}

void ContestController::deleteContest(std::string contestId) 
{
    auto it = contests.find(contestId);
    if (it != contests.end()) 
    {
        delete it->second;
        contests.erase(it);
        logger->log("Deleted contest: " + contestId);
    }
    else 
    {
        logger->log("Contest not found to delete: " + contestId);
    }
}

Contest* ContestController::getContest(std::string contestId) 
{
    if (contests.find(contestId) != contests.end()) 
    {
        return contests[contestId];
    }
    return nullptr;
}

Scoreboard* ContestController::getScoreboard(std::string contestId)
{
    DBController* db = getDB();
    if (!db)
    {
        logger->log("No DBController found for getScoreboard");
        return nullptr;
    }

    auto results = db->executeQuery(
        "SELECT TeamID, COUNT(*) AS Score FROM SolvedChallenges WHERE ContestID = ? GROUP BY TeamID ORDER BY Score DESC",
        { contestId });

    if (results.empty())
    {
        logger->log("No scores found for contest: " + contestId);
        return nullptr;
    }

    Scoreboard* sb = new Scoreboard(contestId);

    for (const auto& row : results)
    {
        if (row.count("teamId") && row.count("score"))
        {
            try
            {
                std::string teamId = row.at("teamId");
                int score = std::stoi(row.at("score"));
                sb->getScores().emplace_back(teamId, score);  // direct push în vector
            }
            catch (const std::exception& e)
            {
                logger->log("Error parsing score row: " + std::string(e.what()));
            }
        }
    }

    logger->log("Generated scoreboard for contest: " + contestId);
    return sb;
}

DBController* ContestController::getDB() 
{
    return dbController;
}
