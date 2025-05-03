#pragma once
#include <string>
#include <vector>
#include "ChallTypes.h"
#include "Orchestrator.h"

class Chall {
protected:
    int id;
    std::string name;
    int difficulty;
    std::vector<ChallTypes> types;
    std::string description;
    std::string question;
    int points;
    std::string flag;
    std::vector<std::string> hints;
    std::string authorId;
    std::vector<Orchestrator*> orchestrators;
    std::vector<std::string> filePaths;

public:
    Chall(const std::string& name, const std::vector<ChallTypes>& types,int id=0);
    virtual ~Chall() = default;

    int getId() const;
    std::string getName() const;
    int getDifficulty() const;
    std::vector<ChallTypes> getTypes() const;
    std::string getDescription() const;
    std::string getQuestion() const;
    int getPoints() const;
    std::string getFlag() const;
    std::vector<std::string> getHints() const;
    std::string getAuthor() const;
    std::vector<Orchestrator*> getOrchestrators() const;
    std::vector<std::string> getFilePaths() const;

    void setDifficulty(int diff);
    void setDescription(const std::string& desc);
    void setQuestion(const std::string& ques);
    void setPoints(int pts);
    void setFlag(const std::string& flg);
    void addHint(const std::string& hint);
    void setAuthor(const std::string& author);
    void addOrchestrator(Orchestrator* orch);
    void removeOrchestrator(int orchId);
    void addFilePath(const std::string& path);

    virtual bool validateFlag(const std::string& attemptFlag) const;
};