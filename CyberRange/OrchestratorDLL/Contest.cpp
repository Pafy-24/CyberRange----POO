#include"pch.h"
#include "Contest.h"
#include <random>
#include <algorithm>

Contest::Contest(const std::string& name,int id) : name(name), active(false), startTime(0), endTime(0),id(id) {
}

int Contest::getId() const { return id; }
std::string Contest::getName() const { return name; }
time_t Contest::getStartTime() const { return startTime; }
time_t Contest::getEndTime() const { return endTime; }
std::vector<std::string> Contest::getChallenges() const { return challIds; }
std::vector<std::string> Contest::getTeams() const { return teamIds; }
bool Contest::isActive() const { return active; }

void Contest::setStartTime(time_t start) { startTime = start; }
void Contest::setEndTime(time_t end) { endTime = end; }
void Contest::setActive(bool act) { active = act; }

void Contest::addChallenge(const std::string& challId) {
    if (std::find(challIds.begin(), challIds.end(), challId) == challIds.end()) {
        challIds.push_back(challId);
    }
}

void Contest::removeChallenge(const std::string& challId) {
    challIds.erase(std::remove(challIds.begin(), challIds.end(), challId), challIds.end());
}

void Contest::addTeam(const std::string& teamId) {
    if (std::find(teamIds.begin(), teamIds.end(), teamId) == teamIds.end()) {
        teamIds.push_back(teamId);
    }
}

void Contest::removeTeam(const std::string& teamId) {
    teamIds.erase(std::remove(teamIds.begin(), teamIds.end(), teamId), teamIds.end());
}