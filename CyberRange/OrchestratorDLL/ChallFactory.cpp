#include "pch.h"
#include "ChallFactory.h"
#include <stdexcept>

Chall* ChallFactory::CreateFromRow(const std::map<std::string, std::string>& row) 
{
    std::string name = row.at("title");
    int id = std::stoi(row.at("challId"));
    int difficulty = std::stoi(row.at("difficulty"));
    int points = std::stoi(row.at("points"));
    std::string flag = row.at("flag");
    std::string author = row.at("authorId");
    std::string typeStr = row.at("type");

    ChallTypes type;

    if (typeStr == "Crypto") 
        type = ChallTypes::Crypto;
    else if (typeStr == "Pwn") 
        type = ChallTypes::Pwn;
    else if (typeStr == "Web") 
        type = ChallTypes::Web;
    else if (typeStr == "Forensics") 
        type = ChallTypes::Forensics;
    else 
        type = ChallTypes::Misc;

    Chall* chall = new Chall(name, { type }, id);
    chall->setDifficulty(difficulty);
    chall->setPoints(points);
    chall->setFlag(flag);
    chall->setAuthor(author);

    if (row.count("description")) 
        chall->setDescription(row.at("description"));
    if (row.count("question")) 
        chall->setQuestion(row.at("question"));

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
            // log la nevoie
        }
    }
    return result;
}