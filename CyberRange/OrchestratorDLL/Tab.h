#pragma once
#include <string>
#include <vector>

class Tab {
private:
    int id;
    std::string name;
    std::vector<std::string> challIds;
    bool visible;

public:
    Tab(const std::string& name,int id=0);
    int getId() const;
    std::string getName() const;
    std::vector<std::string> getChallenges() const;
    bool isVisible() const;

    void addChallenge(const std::string& challId);
    void removeChallenge(const std::string& challId);
    void setVisible(bool vis);
};