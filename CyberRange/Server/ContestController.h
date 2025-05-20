#pragma once
#include "CController.h"
#include "Observable.h"
#include "Contest.h"
#include <string>

class Connection;

class ContestController : public CController, public Observable {
public:
    ContestController();

    void createContest(const std::string& contestData, Connection* conn);
    void updateContest(const std::string& contestId, const std::string& contestData, Connection* client);
    void deleteContest(const std::string& contestId, Connection* client);
    void sendContestDetails(int contestId, Connection* client);
    void sendScoreboard(int contestId, Connection* client);
    void sendAllContests(Connection* client);

    void handleRequest(const std::string& data, Connection* client) override;
};