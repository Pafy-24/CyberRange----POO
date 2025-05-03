#include "pch.h"
#include "Chall.h"
#include <algorithm>
#include <random>
#include <ctime>

Chall::Chall(const std::string& name, const std::vector<ChallTypes>& types,int id)
    : name(name), types(types), difficulty(1), points(100), authorId("unknown"),id(id) {
    
}

int Chall::getId() const { return id; }
std::string Chall::getName() const { return name; }
int Chall::getDifficulty() const { return difficulty; }
std::vector<ChallTypes> Chall::getTypes() const { return types; }
std::string Chall::getDescription() const { return description; }
std::string Chall::getQuestion() const { return question; }
int Chall::getPoints() const { return points; }
std::string Chall::getFlag() const { return flag; }
std::vector<std::string> Chall::getHints() const { return hints; }
std::string Chall::getAuthor() const { return authorId; }
std::vector<Orchestrator*> Chall::getOrchestrators() const { return orchestrators; }
std::vector<std::string> Chall::getFilePaths() const { return filePaths; }

void Chall::setDifficulty(int diff) { difficulty = diff; }
void Chall::setDescription(const std::string& desc) { description = desc; }
void Chall::setQuestion(const std::string& ques) { question = ques; }
void Chall::setPoints(int pts) { points = pts; }
void Chall::setFlag(const std::string& flg) { flag = flg; }
void Chall::addHint(const std::string& hint) { hints.push_back(hint); }
void Chall::setAuthor(const std::string& author) { authorId = author; }
void Chall::addOrchestrator(Orchestrator* orch) { orchestrators.push_back(orch); }
void Chall::addFilePath(const std::string& path) { filePaths.push_back(path); }

void Chall::removeOrchestrator(int orchId) {
    orchestrators.erase(
        std::remove_if(orchestrators.begin(), orchestrators.end(),
            [&orchId](const Orchestrator* orch) { return orch->getId() == orchId; }),
        orchestrators.end());
}

bool Chall::validateFlag(const std::string& attemptFlag) const {
    return attemptFlag == flag;
}