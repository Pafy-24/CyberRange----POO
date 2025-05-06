#include "Loader.h"
#include "UserController.h"
#include "UsersFactory.h"
#include "TeamController.h"
#include "ChallController.h"
#include "ContestController.h"
#include "ServerMng.h"
#include <iostream>
#include <algorithm> 

Controller* getController(const std::string& name)
{
    return ServerMng::getInstance()->getController(name);
}

// ---------------------- Load methods ----------------------

void Loader::loadUser(int id, Connection* conn)
{
    auto& users = ServerMng::getInstance()->getUsers();
    auto userEntry = users.find(id);

    if (userEntry == users.end() || userEntry->second.first == nullptr)
    {
        std::string query = "SELECT * FROM Users WHERE UserID = '" + std::to_string(id) + "'";
        auto results = ServerMng::getInstance()->getDBController()->executeQuery(query);

        if (!results.empty())
        {
            User* user = nullptr;
            const auto& result = results[0];
            const std::string& role = result.at("Role");

            if (role == "admin")
            {
                user = UsersFactory::CreateAdmin(result.at("Username"), result.at("Email"), id).release();
            }
            else if (role == "writer")
            {
                user = UsersFactory::CreateWriter(result.at("Username"), result.at("Email"), id).release();
            }
            else
            {
                user = UsersFactory::CreateUser(result.at("Username"), result.at("Email"), id).release();
            }

            if (user)
            {
                user->SetPassword(result.at("PasswordHash"));
                ServerMng::getInstance()->pushUser(user, conn);
                registerObject(conn, "user:" + std::to_string(id));
            }
        }
        else
        {
            std::cerr << "[Loader] User not found: " << id << "\n";
        }
    }
    else
    {
        auto& connections = userEntry->second.second;
        if (std::find(connections.begin(), connections.end(), conn) == connections.end())
        {
            connections.push_back(conn);
        }
    }
}

int Loader::loadUserByUsername(const std::string& username, Connection* conn)
{
    std::string query = "SELECT UserID FROM Users WHERE Username = '" + username + "'";

    auto results = ServerMng::getInstance()->getDBController()->executeQuery(query);
    if (!results.empty())
    {
        loadUser(std::stoi(results[0].at("UserID")), conn);
        return std::stoi(results[0].at("UserID"));
    }
    return -1;
}

int Loader::loadUserByEmail(const std::string& email, Connection* conn)
{
    std::string query = "SELECT UserID FROM Users WHERE Email = '" + email + "'";

    auto results = ServerMng::getInstance()->getDBController()->executeQuery(query);
    if (!results.empty())
    {
        loadUser(std::stoi(results[0].at("UserID")), conn);
        return std::stoi(results[0].at("UserID"));
    }
    return -1;
}

void Loader::loadTeam(int id, Connection* conn)
{
    auto& teams = ServerMng::getInstance()->getTeams();
    auto teamEntry = teams.find(id);

    if (teamEntry == teams.end() || teamEntry->second.first == nullptr)
    {
        std::string query = "SELECT UserID FROM UserTeams WHERE TeamID = '" + std::to_string(id) + "'";
        auto results = ServerMng::getInstance()->getDBController()->executeQuery(query);
        std::vector<int> userIds;

        if (!results.empty())
        {
            for (const auto& result : results)
            {
                userIds.push_back(std::stoi(result.at("UserID")));
                loadUser(userIds.back(), conn);
            }
        }

        query = "SELECT * FROM Teams WHERE TeamID = '" + std::to_string(id) + "'";
        results = ServerMng::getInstance()->getDBController()->executeQuery(query);

        if (!results.empty())
        {
            const auto& result = results[0];
            Team* team = UsersFactory::CreateTeam(result.at("Name"), std::stoi(result.at("ContestID")), id).release();
            for (int userId : userIds)
            {
                team->AddMember(ServerMng::getInstance()->getUsers()[userId].first);
            }
            team->SetLeader(std::stoi(result.at("LeaderID")));

            ServerMng::getInstance()->pushTeam(team, conn);
            registerObject(conn, "team:" + std::to_string(id));
        }
        else
        {
            std::cerr << "[Loader] Team not found: " << id << "\n";
        }
    }
    else
    {
        auto& connections = teamEntry->second.second;
        if (std::find(connections.begin(), connections.end(), conn) == connections.end())
        {
            connections.push_back(conn);
        }
    }
}

void Loader::loadChall(int id, Contest* mng, Connection* conn)
{
    std::string query = "SELECT * FROM Challenges WHERE ChallengeID = '" + std::to_string(id) + "'";
    auto results = ServerMng::getInstance()->getDBController()->executeQuery(query);

    if (!results.empty())
    {
        const auto& result = results[0];
        std::vector<ChallTypes> CT;
        std::string type = result.at("Types");
        if (type.find("Forensics") != std::string::npos) CT.push_back(ChallTypes::Forensics);
        if (type.find("Crypto") != std::string::npos) CT.push_back(ChallTypes::Crypto);
        if (type.find("Pwn") != std::string::npos) CT.push_back(ChallTypes::Pwn);
        if (type.find("Web") != std::string::npos) CT.push_back(ChallTypes::Web);
        if (type.find("Misc") != std::string::npos) CT.push_back(ChallTypes::Misc);

        Chall* chall = new Chall(result.at("Name"), CT, id);
        mng->addChallenge(id, chall);
        registerObject(conn, "chall:" + std::to_string(id));
    }
    else
    {
        std::cerr << "[Loader] Challenge not found: " << id << "\n";
    }
}

void Loader::loadChall(int id, Tab* mng, Connection* conn)
{
    std::string query = "SELECT * FROM Challenges WHERE ChallengeID = '" + std::to_string(id) + "'";
    auto results = ServerMng::getInstance()->getDBController()->executeQuery(query);

    if (!results.empty())
    {
        const auto& result = results[0];
        std::vector<ChallTypes> CT;
        std::string type = result.at("Types");
        if (type.find("Forensics") != std::string::npos) CT.push_back(ChallTypes::Forensics);
        if (type.find("Crypto") != std::string::npos) CT.push_back(ChallTypes::Crypto);
        if (type.find("Pwn") != std::string::npos) CT.push_back(ChallTypes::Pwn);
        if (type.find("Web") != std::string::npos) CT.push_back(ChallTypes::Web);
        if (type.find("Misc") != std::string::npos) CT.push_back(ChallTypes::Misc);

        Chall* chall = new Chall(result.at("Name"), CT, id);
        mng->addChallenge(id, chall);
        registerObject(conn, "chall:" + std::to_string(id));
    }
    else
    {
        std::cerr << "[Loader] Challenge not found: " << id << "\n";
    }
}

void Loader::loadContest(int id, Connection* conn)
{
    auto* contest = ServerMng::getInstance()->getContest(id);
    if (!contest)
    {
        std::string query = "SELECT * FROM Contests WHERE ContestID = '" + std::to_string(id) + "'";
        auto results = ServerMng::getInstance()->getDBController()->executeQuery(query);

        if (!results.empty())
        {
            const auto& result = results[0];
            Contest* contest = new Contest(result.at("Name"), id);

            if (contest)
            {
                ServerMng::getInstance()->getChallMng()->addContest(contest);
                registerObject(conn, "contest:" + std::to_string(id));
            }
        }
        else
        {
            std::cerr << "[Loader] Contest not found: " << id << "\n";
        }
    }
}

void Loader::loadTab(int id, Connection* conn)
{
    auto* tab = ServerMng::getInstance()->getTab(id);
    if (!tab)
    {
        std::string query = "SELECT * FROM Tabs WHERE TabID = '" + std::to_string(id) + "'";
        auto results = ServerMng::getInstance()->getDBController()->executeQuery(query);

        if (!results.empty())
        {
            const auto& result = results[0];
            Tab* tab = new Tab(result.at("Name"), id);

            if (tab)
            {
                ServerMng::getInstance()->getChallMng()->addTab(tab);
                registerObject(conn, "tab:" + std::to_string(id));
            }
        }
        else
        {
            std::cerr << "[Loader] Tab not found: " << id << "\n";
        }
    }
}

// ---------------------- Save/Unload ----------------------

void Loader::save(Connection* conn)
{
    if (objLoaded.count(conn))
    {
        for (const auto& objId : objLoaded[conn])
        {
            saveObject(objId);
        }
    }
    std::cout << "[Loader] Saved all objects for connection.\n";
}

void Loader::unloadObject(Connection* conn, const std::string& objId)
{
    size_t pos = objId.find(':');
    if (pos != std::string::npos)
    {
        std::string type = objId.substr(0, pos);
        std::string id = objId.substr(pos + 1);
        int objId = std::stoi(id);

        if (type == "user")
        {
            cleanupUser(objId, conn);
        }
        else if (type == "team")
        {
            cleanupTeam(objId, conn);
        }

    }
}

void Loader::unload(Connection* conn)
{

    if (objLoaded.count(conn))
    {
        for (const auto& objId : objLoaded[conn])
        {
			unloadObject(conn, objId);
        }
        objLoaded.erase(conn);
        std::cout << "[Loader] Unloaded all objects for connection.\n";
    }
}

void Loader::saveUnload(Connection* conn)
{
    save(conn);
    unload(conn);
}

// ---------------------- Internal helpers ----------------------

void Loader::registerObject(Connection* conn, const std::string& objId)
{
    objLoaded[conn].insert(objId);
    std::cout << "[Loader] Registered object: " << objId << " for connection.\n";
}

bool Loader::isLoaded(Connection* conn, const std::string& objId) const
{
    if (objLoaded.count(conn) == 0) {
        return false;
    }
    return objLoaded.at(conn).find(objId) != objLoaded.at(conn).end();
}

void Loader::saveObject(const std::string& objId)
{
    size_t pos = objId.find(':');
    if (pos != std::string::npos) {
        std::string type = objId.substr(0, pos);
        std::string id = objId.substr(pos + 1);

        if (type == "user") {
            int userId = std::stoi(id);
            auto& users = ServerMng::getInstance()->getUsers();
            if (users.find(userId) != users.end() && users[userId].first != nullptr) {
                User* user = users[userId].first;
                std::string role;
                switch (user->GetAccessLevel()) {
                case 1:
                    role = "student"; break;
                case 5:
                    role = "writer"; break;
                case 10:
                    role = "admin"; break;
                default:
                    role = "student"; break;
                }

                std::string checkQuery = "SELECT COUNT(*) as count FROM Users WHERE UserID = '" + id + "'";
                auto results = ServerMng::getInstance()->getDBController()->executeQuery(checkQuery);

                bool userExists = false;
                if (!results.empty()) {
                    userExists = std::stoi(results[0]["count"]) > 0;
                }

                if (userExists) {
                    std::string query = "UPDATE Users SET Username = '" + user->GetUsername() +
                        "', PasswordHash = '" + user->GetPassword() +
                        "', Email = '" + user->GetEmail() +
                        "', Role = '" + role +
                        "' WHERE UserID = '" + id + "'";
                    ServerMng::getInstance()->getDBController()->executeUpdate(query);
                }
                else {

                    std::string query = "INSERT INTO Users (Username, PasswordHash, Email, Role) VALUES ('" + user->GetUsername() + 
                        "', '" + user->GetPassword() + "', '" + user->GetEmail() + "', '" + role + "')";

                    ServerMng::getInstance()->getDBController()->executeUpdate(query);
                }
                std::cout << "[Loader] Saved user: " << id << "\n";
            }
        }
        else if (type == "team") {
            int teamId = std::stoi(id);
            auto& teams = ServerMng::getInstance()->getTeams();
            if (teams.find(teamId) != teams.end() && teams[teamId].first != nullptr) {
                Team* team = teams[teamId].first;

                std::string checkQuery = "SELECT COUNT(*) as count FROM Teams WHERE TeamID = '" + id + "'";
                auto results = ServerMng::getInstance()->getDBController()->executeQuery(checkQuery);

                bool teamExists = false;
                if (!results.empty()) {
                    teamExists = std::stoi(results[0]["count"]) > 0;
                }

                if (teamExists) {
                    std::string query = "UPDATE Teams SET Name = '" + team->GetName() +
                        "', LeaderID = '" + std::to_string(team->GetLeader()) +
                        "', ContestID = '" + std::to_string(team->GetContestId()) +
                        "' WHERE TeamID = '" + id + "'";
                    ServerMng::getInstance()->getDBController()->executeUpdate(query);
                }
                else {
                    std::string query = "INSERT INTO Teams (TeamID, Name, LeaderID, ContestID) VALUES ('" +
                        id + "', '" + team->GetName() + "', '" + std::to_string(team->GetLeader()) +
                        "', '" + std::to_string(team->GetContestId()) + "')";
                    ServerMng::getInstance()->getDBController()->executeUpdate(query);

                    auto members = team->GetMembers();
                    for (auto* member : members) {
                        std::string relationQuery = "INSERT INTO UserTeams (UserID, TeamID) VALUES ('" +
                            std::to_string(member->GetId()) + "', '" + id + "')";
                        ServerMng::getInstance()->getDBController()->executeUpdate(relationQuery);
                    }
                }
                std::cout << "[Loader] Saved team: " << id << "\n";
            }
        }
        else if (type == "chall") {
            int challId = std::stoi(id);
            // Aici trebuie implementată logica pentru a salva provocările
            // Vom avea nevoie de acces la obiectul challenge și la metodele sale
            std::cout << "[Loader] Challenge save not yet implemented for ID: " << id << "\n";
        }
        else if (type == "contest") {
            int contestId = std::stoi(id);
            auto* contest = ServerMng::getInstance()->getContest(contestId);
            if (contest) {
                std::string checkQuery = "SELECT COUNT(*) as count FROM Contests WHERE ContestID = '" + id + "'";
                auto results = ServerMng::getInstance()->getDBController()->executeQuery(checkQuery);

                bool contestExists = false;
                if (!results.empty()) {
                    contestExists = std::stoi(results[0]["count"]) > 0;
                }

                if (contestExists) {
                    std::string query = "UPDATE Contests SET Name = '" + contest->getName() +
                        "' WHERE ContestID = '" + id + "'";
                    ServerMng::getInstance()->getDBController()->executeUpdate(query);
                }
                else {
                    std::string query = "INSERT INTO Contests (ContestID, Name) VALUES ('" +
                        id + "', '" + contest->getName() + "')";
                    ServerMng::getInstance()->getDBController()->executeUpdate(query);
                }
                std::cout << "[Loader] Saved contest: " << id << "\n";
            }
        }
        else if (type == "tab") {
            int tabId = std::stoi(id);
            auto* tab = ServerMng::getInstance()->getTab(tabId);
            if (tab) {
                std::string checkQuery = "SELECT COUNT(*) as count FROM Tabs WHERE TabID = '" + id + "'";
                auto results = ServerMng::getInstance()->getDBController()->executeQuery(checkQuery);

                bool tabExists = false;
                if (!results.empty()) {
                    tabExists = std::stoi(results[0]["count"]) > 0;
                }

                if (tabExists) {
                    std::string query = "UPDATE Tabs SET Name = '" + tab->getName() +
                        "' WHERE TabID = '" + id + "'";
                    ServerMng::getInstance()->getDBController()->executeUpdate(query);
                }
                else {
                    std::string query = "INSERT INTO Tabs (TabID, Name) VALUES ('" +
                        id + "', '" + tab->getName() + "')";
                    ServerMng::getInstance()->getDBController()->executeUpdate(query);
                }
                std::cout << "[Loader] Saved tab: " << id << "\n";
            }
        }
    }
}


void Loader::cleanupUser(int userId, Connection* conn)
{
    auto& users = ServerMng::getInstance()->getUsers();
    auto it = users.find(userId);
    if (it != users.end())
    {
        auto& connections = it->second.second;
        connections.remove_if([&](Connection* connection) { return connection == conn || !connection->isConnected(); });
        if (connections.empty())
        {
            delete it->second.first;
            users.erase(it);
            std::cout << "[Loader] Cleaned up user: " << userId << "\n";
        }
    }
}

void Loader::cleanupTeam(int teamId, Connection* conn)
{
    auto& teams = ServerMng::getInstance()->getTeams();
    auto it = teams.find(teamId);
    if (it != teams.end())
    {
        auto& connections = it->second.second;
        connections.remove_if([&](Connection* connection) { return connection == conn || !connection->isConnected(); });
        if (connections.empty())
        {
            delete it->second.first;
            teams.erase(it);
            std::cout << "[Loader] Cleaned up team: " << teamId << "\n";
        }
    }
}