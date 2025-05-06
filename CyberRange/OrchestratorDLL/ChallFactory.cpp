#include "pch.h"
#include "ChallFactory.h"
#include <stdexcept>

Chall* ChallFactory::CreateFromRow(const std::map<std::string, std::string>& row) 
{
    std::string name = row.at("Name");
    int id = std::stoi(row.at("ChallengeID"));
    std::string difficulty = row.at("Difficulty");
    int points = std::stoi(row.at("Score"));
    std::string flag = row.at("Flag");
    std::string author = row.at("AuthorId");
    std::string typesStr = row.at("Tags");

    std::vector<ChallTypes> types;
	for (const auto& typePair : challTypeMap) {
		if (typesStr.find(typePair.first) != std::string::npos) {
			types.push_back(typePair.second);
		}
	}

    Chall* chall = new Chall(name, types, id);
    chall->setDifficulty(difficulty);
    chall->setPoints(points);
    chall->setFlag(flag);
    chall->setAuthor(std::stoi(author));
    chall->setDescription(row.at("Description"));
	chall->addFilePath(row.at("FilePath"));
    return chall;
}

std::vector<Chall*> ChallFactory::CreateFromList(const std::vector<std::map<std::string, std::string>>& rows) 
{
    std::vector<Chall*> result;
    for (const auto& row : rows) {
        try {
            result.push_back(CreateFromRow(row));
        }
        catch (const std::exception& e) {
            
        }
    }
    return result;
}