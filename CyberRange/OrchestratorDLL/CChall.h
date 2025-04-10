#pragma once
#include <string>
#include <vector>
#include "Chall.h"
#include "ChallTypes.h"

class CChall : public Chall {
private:
    std::string id;
    std::string name;
    int difficulty;
    ChallTypes type;
    std::string text;
    int points;
    std::string flag;
    std::vector<std::string> hints;
    std::string authorId;

public:
    CChall(std::string name, ChallTypes type);
    std::string getId() override;
    std::string getName() override;
    int getDifficulty() override;
    ChallTypes getType() override;
    std::string getText() override;
    int getPoints() override;
    bool validateFlag(std::string attemptFlag);
    void addHint(std::string hint);
    std::vector<std::string> getHints();
    void setAuthor(std::string authorId);
    std::string getAuthor();
};
