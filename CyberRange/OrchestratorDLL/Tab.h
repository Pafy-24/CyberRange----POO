#pragma once
#include "DLL.h"
#include "Chall.h"

class ORCHESTRATOR_API Tab {
private:
    int id;
    std::string name;
    std::map<int, Chall*> challs;
public:
    Tab(const std::string& name,int id=0);
    int getId() const;
    std::string getName() const;
    std::map<int, Chall*> getChallenges() const;

    void addChallenge(int challId, Chall* chall);
    void removeChallenge(int challId);
};