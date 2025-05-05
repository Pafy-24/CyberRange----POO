#include "Contest.h"

Contest::Contest(const std::string& name, int id) :
    name(name),
    id(id),
    startTime(0),
    endTime(0),
    active(false) {
}

int Contest::getId() const {
    return id;
}

std::string Contest::getName() const {
    return name;
}

time_t Contest::getStartTime() const {
    return startTime;
}

time_t Contest::getEndTime() const {
    return endTime;
}

std::map<int, Chall*> Contest::getChallenges() const {
    return challs;
}

std::vector<int> Contest::getTeams() const {
    return teamIds;
}

bool Contest::isActive() const {
    return active;
}

void Contest::setStartTime(time_t start) {
    startTime = start;
}

void Contest::setEndTime(time_t end) {
    endTime = end;
}

void Contest::addChallenge(int challId, Chall* chall) {
    if (challs.find(challId) == challs.end()) {
		challs[challId] = chall;
	}
}

void Contest::removeChallenge(int challId) {
    challs.erase(challId);
}

void Contest::addTeam(int teamId) {
    if (std::find(teamIds.begin(), teamIds.end(), teamId) == teamIds.end()) {
        teamIds.push_back(teamId);
    }
}

void Contest::removeTeam(int teamId) {
    auto it = std::find(teamIds.begin(), teamIds.end(), teamId);
    if (it != teamIds.end()) {
        teamIds.erase(it);
    }
}

void Contest::setActive(bool act) {
    active = act;
}