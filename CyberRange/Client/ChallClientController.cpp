#include "ChallClientController.h"
#include "ClientMng.h"
#include "ChallMng.h"
#include "ChallFactory.h"
#include "json.hpp"

using json = nlohmann::json;

ChallClientController::ChallClientController() : CController("Chall") {}

void ChallClientController::requestChallengeList() 
{
    json req = 
    {
        {"controller", "Chall"},
        {"action", "getChallengeList"},
        {"payload", json::object()}
    };
    ClientMng::getInstance()->sendRequest(req.dump());
}

void ChallClientController::requestChallengeDetails(const std::string& challId) 
{
    json req = 
    {
        {"controller", "Chall"},
        {"action", "getChallengeDetails"},
        {"payload", { {"challId", challId} }}
    };
    ClientMng::getInstance()->sendRequest(req.dump());
}

void ChallClientController::submitFlag(const std::string& challId, const std::string& flag) 
{
    json req = {
        {"controller", "Chall"},
        {"action", "submitFlag"},
        {"payload", {
            {"challId", challId},
            {"flag", flag}
        }}
    };
    ClientMng::getInstance()->sendRequest(req.dump());
}

void ChallClientController::handleServerResponse(const std::string& responseStr) 
{
    try {
        json response = json::parse(responseStr);

        // Verificare câmpuri esențiale
        if (!response.contains("action") || !response.contains("status"))
        {
            std::cerr << "[ChallClientController] Invalid response structure.\n";
            return;
        }

        std::string action = response["action"];
        std::string status = response["status"];

        // Tratare getChallengeList
        if (action == "getChallengeList")
        {
            if (status == "success" && response.contains("data"))
            {
                auto challData = response["data"];

                std::vector<std::map<std::string, std::string>> parsed;
                for (const auto& item : challData)
                {
                    std::map<std::string, std::string> row;
                    for (auto it = item.begin(); it != item.end(); ++it)
                    {
                        row[it.key()] = it.value().get<std::string>();
                    }
                    parsed.push_back(row);
                }

                ChallMng::getInstance()->loadChallengesFromData(parsed);
                std::cout << "[ChallClientController] Challenges loaded: " << parsed.size() << "\n";
            }
            else
            {
                std::cerr << "[ChallClientController] Failed to load challenge list.\n";
            }
        }

        // Tratare submitFlag
        else if (action == "submitFlag")
        {
            std::string message = response.value("message", "No message");

            if (status == "success")
            {
                std::cout << "[ChallClientController] Flag correct: " << message << "\n";
                // TODO: notificare UI
            }
            else
            {
                std::cout << "[ChallClientController] Flag incorrect: " << message << "\n";
                // TODO: notificare UI
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ChallClientController] JSON parse error: " << e.what() << "\n";
    }
}