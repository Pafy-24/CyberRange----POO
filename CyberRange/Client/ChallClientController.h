#pragma once
#include "CController.h"
#include <string>
#include <QObject>

class ChallClientController : public QObject, public CController{
	Q_OBJECT
public:
    ChallClientController();

    void requestChallengeList();
	void requestAddChallenge();
    void requestChallengeDetails(const int& challId);
    void submitFlag(const int& challId, const std::string& flag);
    void handleServerResponse(const std::string& response) override;

signals:
	void loadedChallenges();
	void loadedChallengeDetails(std::string a, std::string b, std::string c, std::string d, std::string e, std::string f, std::string g);
	void flagSubmitted(const std::string& response);
	void flagSubmissionFailed(const std::string& error);
};