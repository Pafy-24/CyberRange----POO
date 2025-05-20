#include "TabController.h"
#include "ServerMng.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

TabController::TabController()
    : CController("TabController") {
}

void TabController::handleRequest(const std::string& data, Connection* client) {
    if (!validateRequest(data, client, 1)) {
        return;
    }

    auto dbController = ServerMng::getInstance()->getDBController();

    try {
        json j = json::parse(data);
        std::string action = j["action"].get<std::string>();

        if (action == "getTabList") 
        {
            std::string query = "SELECT * FROM C";
            auto results = dbController->executeQuery(query);

            if (!results.empty()) {
                json data = json::array();
                for (const auto& row : results) {
                    data.push_back({
                        {"tabId", row.at("TabID")},
                        {"name", row.at("Name")}
                        });
                }

                client->send(json{
                    {"status", "success"},
                    {"action", "getTabList"},
                    {"data", data}
                    }.dump());
                print("Tab list retrieved successfully.");
            }
            else {
                client->send(json{
                    {"status", "error"},
                    {"message", "No tabs found"}
                    }.dump());
                print("No tabs found in database.");
            }
        }
        else if (action == "getTabDetails")
        {
			std::string tabId = j["tabId"].get<std::string>();
			std::string query = "SELECT * FROM Tabs WHERE TabID = ?";
			auto results = dbController->executeQuery(query, { tabId });
			if (!results.empty()) {
				json data = {
					{"tabId", results[0].at("TabID")},
					{"name", results[0].at("Name")}
				};
				client->send(json{
					{"status", "success"},
					{"action", "getTabDetails"},
					{"data", data}
					}.dump());
				print("Tab details retrieved successfully.");
			}
			else {
				client->send(json{
					{"status", "error"},
					{"message", "Tab not found"}
					}.dump());
				print("Tab not found: " + tabId);
			}
        }
        else {
            client->send(json{
                {"status", "error"},
                {"message", "Invalid action"}
                }.dump());
            print("Invalid action in TabController: " + action);
        }
    }
    catch (const std::exception& e) {
        client->send(json{
            {"status", "error"},
            {"message", "Request processing failed"}
            }.dump());
        print("TabController error: " + std::string(e.what()));
    }
}

