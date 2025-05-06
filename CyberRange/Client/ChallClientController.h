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
    void requestChallengeDetails(const std::string& challId);
    void submitFlag(const std::string& challId, const std::string& flag);
    void handleServerResponse(const std::string& response) override;

signals:
	void loadedChallenges();
	void loadedChallengeDetails();
	void flagSubmitted(const std::string& response);
	void flagSubmissionFailed(const std::string& error);
};