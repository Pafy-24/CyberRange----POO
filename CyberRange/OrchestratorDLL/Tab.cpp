#include "pch.h"
#include "Tab.h"

Tab::Tab(const std::string& name, int id) :
    name(name),
    id(id) {
}

int Tab::getId() const {
    return id;
}

std::string Tab::getName() const {
    return name;
}

std::map<int, Chall*> Tab::getChallenges() const {
    return challs;
}

void Tab::addChallenge(int challId, Chall* chall) {
    if (challs.find(challId) == challs.end()) {
		challs[challId] = chall;
    }
}

void Tab::removeChallenge(int challId) {
    challs.erase(challId);
}