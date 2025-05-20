#pragma once
#include "CController.h"
#include <QObject>
#include <string>

class TabClientController : public QObject, public CController {
    Q_OBJECT
public:
    TabClientController();
    void requestTabList();
    void handleServerResponse(const std::string& response) override;
signals:
	void loadedTabs();
	void loadedTabDetails();
};