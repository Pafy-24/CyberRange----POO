#include "TabClientController.h"
#include "ClientMng.h"
#include "json.hpp"
#include <QTimer>
using json = nlohmann::json;

TabClientController::TabClientController() : CController("Tab") {}

void TabClientController::requestTabList() {
    json req = {
        {"controller", "TabController"},
        {"token", ClientMng::getInstance()->getAuthToken()},
        {"action", "getTabList"},
        {"payload", json::object()}
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

void TabClientController::requestTabDetails(const std::string& tabId)
{
	json req = {
		{"controller", "TabController"},
		{"token", ClientMng::getInstance()->getAuthToken()},
		{"action", "getTabDetails"},
		{"payload", { {"tabId", tabId} }}
	};
	ClientMng::getInstance()->sendRequest(req.dump());
}

void TabClientController::handleServerResponse(const std::string& responseStr) {
    try {
        json response = json::parse(responseStr);
        std::string action = response["action"];
        if (action == "getTabList" && response["status"] == "success") {
            auto data = response["data"];
            // TODO: salvează în TabManager sau trimite spre UI
            qDebug() << "[TabClientController] Loaded " << data.size() << " tabs.";
        }
    }
    catch (...) {
        qWarning() << "[TabClientController] Invalid tab response.";
    }
}