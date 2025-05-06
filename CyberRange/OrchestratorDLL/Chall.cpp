#include "pch.h"
#include "Chall.h"
#include <algorithm>
#include <random>
#include <ctime>

Chall::Chall(const std::string& name, const std::vector<ChallTypes>& types,int id)
    : name(name), types(types), difficulty(1), points(100), authorId(0),id(id) {
    
}

int Chall::getId() const { return id; }
std::string Chall::getName() const { return name; }
int Chall::getDifficulty() const { return difficulty; }
std::string Chall::getDiffStr() const
{
    switch (difficulty)
    {
    case 1:
        return "easy";
    case 2:
        return "medium";
    case 3:
        return "hard";
    case 4:
        return "very hard";
    case 5:
        return "impossible";
    default:
        return "unknown";
    }
}

std::string Chall::getTags() const
{
    std::string tags = "";
    for (auto type : types) {
		for (auto typePair : challTypeMap) {
			if (type == typePair.second) {
				tags += typePair.first + ";";
				break;
			}
		}
    }
    return tags;
}

std::vector<ChallTypes> Chall::getTypes() const { return types; }
std::string Chall::getDescription() const { return description; }
int Chall::getPoints() const { return points; }
std::string Chall::getFlag() const { return flag; }
std::vector<std::string> Chall::getHints() const { return hints; }
int Chall::getAuthor() const { return authorId; }
std::vector<Orchestrator*> Chall::getOrchestrators() const { return orchestrators; }
std::string Chall::getFilesPath() const { return filesPath; }

void Chall::setDifficulty(int diff) { difficulty = diff; }
void Chall::setDifficulty(const std::string& diff) { 
	if (diff == "easy" || diff=="Easy") difficulty = 1;
	else if (diff == "medium" || diff=="Medium") difficulty = 2;
	else if (diff == "hard" || diff=="Hard") difficulty = 3;
	else if (diff == "very hard" || diff == "Very Hard") difficulty = 4;
	else if (diff == "impossible" || diff == "Impossible") difficulty = 5;
	else throw std::invalid_argument("Invalid difficulty level");
}
void Chall::setDescription(const std::string& desc) { description = desc; }
void Chall::setPoints(int pts) { points = pts; }
void Chall::setAuthor(int author) { authorId = author; }
void Chall::addOrchestrator(Orchestrator* orch) { orchestrators.push_back(orch); }
void Chall::addFilePath(const std::string& path) { filesPath=path; }

void Chall::removeOrchestrator(int orchId) {
    orchestrators.erase(
        std::remove_if(orchestrators.begin(), orchestrators.end(),
            [&orchId](const Orchestrator* orch) { return orch->getId() == orchId; }),
        orchestrators.end());
}

bool Chall::validateFlag(const std::string& attemptFlag) const {
    return attemptFlag == flag;
}
