#include "ChallClientController.h"
#include "ClientMng.h"
#include "ChallMng.h"
#include "ChallFactory.h"
#include "json.hpp"
#include <QDialog>
#include <QTimer>

using json = nlohmann::json;

ChallClientController::ChallClientController() : CController("Chall") {}

void ChallClientController::requestChallengeList() 
{
    json req = {
        {"controller", "ChallController"},
        {"token", ClientMng::getInstance()->getAuthToken()},
        {"action", "getChallengeList"},
        {"payload", json::object()}
    };

    try {
        ClientMng::getInstance()->sendRequest(req.dump());

        QTimer::singleShot(500, [this]() {
            if (ClientMng::getInstance()->isConnected()) {
                ClientMng::getInstance()->receiveResponse();
            }
            else {
                qWarning() << "[ChallClientController] Failed contest request";
            }
            });
    }
    catch (const std::exception& e) {
        qWarning() << "[ChallClientController] Error contest request";
    }
}

void ChallClientController::requestAddChallenge()
{
}

void ChallClientController::requestChallengeDetails(const int& challId) 
{
    json req = 
    {
        {"controller", "ChallController"},
        {"action", "getChallengeDetails"},
		{"token", ClientMng::getInstance()->getAuthToken()},
        {"challId", challId},
        {"tabId", 1}
    };
    ClientMng::getInstance()->sendRequest(req.dump());
	ClientMng::getInstance()->receiveResponse();
}

void ChallClientController::submitFlag(const int& challId, const std::string& flag) 
{
    json req = {
        {"controller", "ChallController"},
        {"action", "submitFlag"},
        {"challId", challId},
        {"flag", flag}
    };
    ClientMng::getInstance()->sendRequest(req.dump());
    ClientMng::getInstance()->receiveResponse();
}

void ChallClientController::handleServerResponse(const std::string& responseStr) 
{
    try {
        json response = json::parse(responseStr);

        if (!response.contains("action") || !response.contains("status"))
        {
            std::cerr << "[ChallClientController] Invalid response structure.\n";
            return;
        }

        std::string action = response["action"];
        std::string status = response["status"];

        if (action == "getChallengeList")
        {
            if (status == "success" && response.contains("data"))
            {
                auto data = response["data"];
                for (const auto& c : data)
                {
                    int challId = c["challId"];
                    std::string title = c["title"];
                    std::string difficulty = c["difficulty"];
                    int score = c["score"];
                    std::string tags = c["tags"];
                    std::vector<ChallTypes> tagList;

                    std::stringstream ss(tags);
                    std::string tag;
                    while (std::getline(ss, tag, ',')) {
                        tag.erase(0, tag.find_first_not_of(" \t\n\r"));
                        tag.erase(tag.find_last_not_of(" \t\n\r") + 1);

                        auto it = challTypeMap.find(tag);
                        if (it != challTypeMap.end()) 
                        {
                            tagList.push_back(it->second);
                        }
                        else 
                        {
                            qWarning() << "[ChallClientController] Unknown tag: " << QString::fromStdString(tag);
                        }
                    }
                    int contestId = std::stoi(c["contestId"].get<std::string>());

                    Chall* challenge = new Chall(title, tagList, challId);
                    challenge->setDifficulty(difficulty);
                    challenge->setScore(score);
                    challenge->setContestId(contestId);

                    ClientMng::getInstance()->getChallMng()->addChallenge(contestId,challId,challenge);
                }

                std::cout << "[ChallClientController] Challenge list loaded.\n";
                emit loadedChallenges();  
            }
            else
            {
                std::cerr << "[ChallClientController] Failed to load challenge list.\n";
            }
        }
        else if (action == "submitFlag")
        {
            std::string message = response.value("message", "No message");

            if (status == "success")
            {
                std::cout << "[ChallClientController] Flag correct: " << message << "\n";
				emit flagSubmitted(message);
            }
            else
            {
                std::cout << "[ChallClientController] Flag incorrect: " << message << "\n";
				emit flagSubmissionFailed(message);
            }
        }
        else if (action == "getChallengeDetails")
        {
            /*
            {"data", {
                        {"challId", challId},
                        {"title", challenge->getName()},
						{"description", challenge->getDescription()},
						{"author", ServerMng::getInstance()->getUsers()[challenge->getAuthor()].first->GetUsername()},
						{"points", challenge->getPoints()},
						{"tags", challenge->getTags()},
                        {"difficulty", challenge->getDiffStr()}
                    }}
            */
			if (status == "success" && response.contains("data"))
			{
				auto challData = response["data"];
				int challId = challData["challId"];
				std::string title = challData["title"];
				std::string description = challData["description"];
				std::string author = challData["author"];
				int points = challData["points"];
				std::string tags = challData["tags"];
				std::string difficulty = challData["difficulty"];

				emit loadedChallengeDetails(std::to_string(challId), title, description, author, std::to_string(points), tags, difficulty);
				std::cout << "[ChallClientController] Challenge details loaded " << "\n";
			}
			else
			{
				std::cerr << "[ChallClientController] Failed to load challenge details.\n";
			}
		}
        else
        {
            std::cerr << "[ChallClientController] Unknown action: " << action << "\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ChallClientController] JSON parse error: " << e.what() << "\n";
    }
}