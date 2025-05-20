#include "TabClientController.h"
#include "ClientMng.h"
#include "json.hpp"
#include <QTimer>
#include <QDebug>
#include "Chall.h"
using json = nlohmann::json;

TabClientController::TabClientController() : CController("TabClientController") {}

void TabClientController::requestTabList() {
    json req = {
        {"controller", "TabController"},
        {"token", ClientMng::getInstance()->getAuthToken()},
        {"action", "getTabDetails"},
        {"tabId", 1}
    };

    try {
        ClientMng::getInstance()->sendRequest(req.dump());
        QTimer::singleShot(500, [] {
            if (ClientMng::getInstance()->isConnected()) {
                ClientMng::getInstance()->receiveResponse();
            }
            });
    }
    catch (...) {
        qWarning() << "[TabClientController] Failed to request tabs.";
    }
}

void TabClientController::handleServerResponse(const std::string& responseStr) {
    try {
        json response = json::parse(responseStr);
        std::string action = response["action"];

        if (action == "getTabDetails" && response["status"] == "success") 
        {
            auto data = response["data"];
            int tabId = data["tabId"];
            std::string name = data["name"];

            Tab* tab = new Tab(name, tabId);

            // Parse challenges
            std::map<int, std::string> challs = data["challs"];
            for (auto& c : challs) 
            {
                int challId = c.first; 
                std::string challName = c.second;
                challs[challId] = challName;
                
                tab->addChallenge(challId, new Chall(challId, challName));
            }
            ClientMng::getInstance()->getChallMng()->addTab(tab);

            qDebug() << "[TabClientController] Parsed tab details for tabId" << tabId;
            emit loadedTabs(); // semnal pentru MainMenu
        }
    }
    catch (...) {
        qWarning() << "[TabClientController] Invalid tab response.";
    }
}