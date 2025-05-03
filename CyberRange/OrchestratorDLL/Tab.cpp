#include"pch.h"
#include "Tab.h"
#include <random>
#include <algorithm>

Tab::Tab(const std::string& name, int id) : name(name), visible(true),id(id) {
}

int Tab::getId() const { return id; }
std::string Tab::getName() const { return name; }
std::vector<std::string> Tab::getChallenges() const { return challIds; }
bool Tab::isVisible() const { return visible; }

void Tab::setVisible(bool vis) { visible = vis; }

void Tab::addChallenge(const std::string& challId) {
    if (std::find(challIds.begin(), challIds.end(), challId) == challIds.end()) {
        challIds.push_back(challId);
    }
}

void Tab::removeChallenge(const std::string& challId) {
    challIds.erase(std::remove(challIds.begin(), challIds.end(), challId), challIds.end());
}