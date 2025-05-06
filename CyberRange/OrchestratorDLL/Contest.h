#pragma once
#include "DLL.h"
#include "Chall.h"

class ORCHESTRATOR_API Contest {
private:
    int id;
    std::string name;
    std::string description;
    time_t startTime;
    time_t endTime;
    std::map<int,Chall*> challs;
    std::vector<int> teamIds;
    bool active;

public:
    Contest(const std::string& name,int id=0);
    int getId() const;
    std::string getName() const;
    std::string getDescription() const;
    time_t getStartTime() const;
    time_t getEndTime() const;
    std::map<int, Chall*> getChallenges() const;
    std::vector<int> getTeams() const;
    bool isActive() const;

	void setName(const std::string& name);
	void setDescription(const std::string& desc);

    void setStartTime(time_t start);
    void setEndTime(time_t end);
    void addChallenge(int challId, Chall* chall);
    void removeChallenge(int challId);
    void addTeam(int teamId);
    void removeTeam(int teamId);
    void setActive(bool act);
};