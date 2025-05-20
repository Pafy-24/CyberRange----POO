#pragma once
#include "CController.h"
#include "DBController.h"
#include "Tab.h"

class TabController : public CController {
private:
	std::map<std::string, Tab*> tabs;
public:
    TabController();
    void handleRequest(const std::string& data, Connection* client) override;
	//void loadTab(const std::string& tabId);
};
