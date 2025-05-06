#pragma once
#include <string>
#include <vector>
#include "ChallTypes.h"
#include "Orchestrator.h"

class ORCHESTRATOR_API Chall {
protected:
    int id;
    std::string name;
    int difficulty;
    std::vector<ChallTypes> types;
    std::string description;
    int points;
    std::string flag;
    std::vector<std::string> hints;
    int authorId;
    std::vector<Orchestrator*> orchestrators;
    std::string filesPath;

public:
    Chall(const std::string& name, const std::vector<ChallTypes>& types,int id=0);
    virtual ~Chall() = default;

    int getId() const;
    std::string getName() const;
    int getDifficulty() const;
    std::string getDiffStr() const;
	std::string getTags() const;
    std::vector<ChallTypes> getTypes() const;
    std::string getDescription() const;
    int getPoints() const;
    std::string getFlag() const;
    std::vector<std::string> getHints() const;
    int getAuthor() const;
    std::vector<Orchestrator*> getOrchestrators() const;
    std::string getFilesPath() const;

    void setDifficulty(int diff);
    void setDifficulty(const std::string& diff);
    void setDescription(const std::string& desc);
    void setPoints(int pts);
    void setFlag(const std::string& flg);
    void setAuthor(int author);
    void addOrchestrator(Orchestrator* orch);
    void removeOrchestrator(int orchId);
    void addFilePath(const std::string& path);

    virtual bool validateFlag(const std::string& attemptFlag) const;
};