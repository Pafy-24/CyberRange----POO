#pragma once
#include <string>
#include <vector>
#include <ctime>

class Contest {
private:
    int id;
    std::string name;
    time_t startTime;
    time_t endTime;
    std::vector<std::string> challIds;
    std::vector<std::string> teamIds;
    bool active;

public:
    Contest(const std::string& name,int id=0);
    int getId() const;
    std::string getName() const;
    time_t getStartTime() const;
    time_t getEndTime() const;
    std::vector<std::string> getChallenges() const;
    std::vector<std::string> getTeams() const;
    bool isActive() const;

    void setStartTime(time_t start);
    void setEndTime(time_t end);
    void addChallenge(const std::string& challId);
    void removeChallenge(const std::string& challId);
    void addTeam(const std::string& teamId);
    void removeTeam(const std::string& teamId);
    void setActive(bool act);
};