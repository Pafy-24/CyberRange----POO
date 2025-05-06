#pragma once
#include "CController.h"
#include <QObject>
#include <string>

class ContestClientController : public QObject, public CController {
    Q_OBJECT
public:
    ContestClientController();

    void requestContestList();
    void requestContestDetails(const int& contestId);
    void requestScoreboard(const std::string& contestId);

    void handleServerResponse(const std::string& responseStr) override;
signals:
    void loadedContests();
    void loadedContestDetails(int id);
};