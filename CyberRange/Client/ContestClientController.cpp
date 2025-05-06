#include "ContestClientController.h"
#include "ClientMng.h"
#include "ChallMng.h"
#include "Contest.h"
#include "Scoreboard.h"
#include "json.hpp"
#include <iostream>
#include <QTimer>
#include <QDebug>

using json = nlohmann::json;

ContestClientController::ContestClientController()
    : CController("Contest") {}

void ContestClientController::requestContestList() 
{
    json req = {
        {"controller", "ContestController"},
		{"token", ClientMng::getInstance()->getAuthToken()},
        {"action", "getContests"},
        {"payload", json::object()}
    };
    
    try {
        ClientMng::getInstance()->sendRequest(req.dump());

        QTimer::singleShot(500, [this]() {
            if (ClientMng::getInstance()->isConnected()) {
                ClientMng::getInstance()->receiveResponse();
            }
            else {
				qWarning() << "[ContestClientController] Failed contest request";
            }
            });
    }
    catch (const std::exception& e) {
        qWarning() << "[ContestClientController] Error contest request";
    }
}

void ContestClientController::requestContestDetails()
{
	json req = {
		{"controller", "ContestController"},
		{"action", "getContest"},
		{"payload", json::object()}
	};
	ClientMng::getInstance()->sendRequest(req.dump());
}

void ContestClientController::requestScoreboard(const std::string& contestId) 
{
    json req = {
        {"controller", "ContestController"},
        {"token", ClientMng::getInstance()->getAuthToken()},
        {"action", "getScoreboard"},
        {"payload", { {"contestId", contestId} }}
    };
    ClientMng::getInstance()->sendRequest(req.dump());
}

void ContestClientController::handleServerResponse(const std::string& responseStr) 
{
    try {
        json response = json::parse(responseStr);

        if (!response.contains("action") || !response.contains("status")) 
        {
            std::cerr << "[ContestClientController] Invalid response format.\n";
            return;
        }

        std::string action = response["action"];
        std::string status = response["status"];

        // Handle getContestList
        if (action == "getContests" && status == "success") 
        {
            auto data = response["contests"];
            for (const auto& c : data) 
            {
                int id = c["contestId"];
                std::string name = c["name"];
                Contest* contest = new Contest(name, id);
				ClientMng::getInstance()->getChallMng()->addContest(contest);
                
            }
            std::cout << "[ContestClientController] Contest list loaded.\n";
			emit loadedContests();
        }
        else if (action == "getContest" && status == "success")
        {
            auto data = response["data"];
            std::string name = data["name"];
            int id = data["contestId"];
            Contest* contest = new Contest(name, id);
			ClientMng::getInstance()->getChallMng()->addContest(contest);
            std::cout << "[ContestClientController] Contest details loaded.\n";
        }
        // Handle getScoreboard
        else if (action == "getScoreboard" && status == "success") 
        {
            std::string contestId = response["contestId"];
            auto scores = response["scoreboard"];
            Scoreboard* sb = new Scoreboard(contestId);

            for (auto it = scores.begin(); it != scores.end(); ++it) 
            {
                std::string teamId = it.key();
                int score = it.value();
                sb->addScore(teamId, score);
            }

            // TODO: afiseaza Scoreboard în orchestrare
            std::cout << "[ContestClientController] Scoreboard received for contest " << contestId << "\n";
        }
		else
		{
			std::cerr << "[ContestClientController] Error: " << response["message"].get<std::string>() << "\n";
		}
    }
    catch (const std::exception& e) {
        std::cerr << "[ContestClientController] Error: " << e.what() << "\n";
    }
}