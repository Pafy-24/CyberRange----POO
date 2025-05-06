#pragma once
#include "CController.h"

class TabClientController : public CController {
public:
    TabClientController();
    void requestTabList();
	void requestTabDetails(const std::string& tabId);
    void handleServerResponse(const std::string& response) override;
};