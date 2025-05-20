#include "Loader.h"
#include "UserController.h"
#include "UsersFactory.h"
#include "TeamController.h"
#include "ChallController.h"
#include "ChallFactory.h"
#include "ContestController.h"
#include "ServerMng.h"
#include <string>
#include <sstream>     // std::istringstream
#include <iomanip>     // std::get_time
#include <ctime>  
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
            printError("[Loader] User not found: " + id );
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
            Team* team = UsersFactory::CreateTeam(result.at("TeamName"), std::stoi(result.at("ContestID")), id).release();
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
            printError("[Loader] Team not found: " + id );
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
        Chall* chall = ChallFactory::CreateFromRow(results[0]);
        mng->addChallenge(id, chall);
        registerObject(conn, "chall:" + std::to_string(id));
    }
    else
    {
        printError("[Loader] Challenge not found: " + id );
    }
}

void Loader::loadChall(int id, Tab* mng, Connection* conn)
{
    std::string query = "SELECT * FROM Challenges WHERE ChallengeID = '" + std::to_string(id) + "'";
    auto results = ServerMng::getInstance()->getDBController()->executeQuery(query);

    if (!results.empty())
    {
        Chall* chall = ChallFactory::CreateFromRow(results[0]);
        mng->addChallenge(id, chall);
        registerObject(conn, "chall:" + std::to_string(id));
    }
    else
    {
        printError("[Loader] Challenge not found: " + id );
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
                auto parseDateTime = [](const std::string& dateTimeStr) -> time_t {
                    std::tm tm = {};
                    std::istringstream ss(dateTimeStr);
                    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
                    if (ss.fail()) {
                        throw std::runtime_error("Failed to parse date-time: " + dateTimeStr);
                    }
                    return std::mktime(&tm);
                };

                contest->setStartTime(parseDateTime(result.at("StartDate"))); // time_t <- yyyy-mm-dd hh:mm:ss.000
                contest->setEndTime(parseDateTime(result.at("EndDate")));     // time_t <- yyyy-mm-dd hh:mm:ss.000
                contest->setDescription(result.at("Description"));
				contest->setMaxTeamUsers(std::stoi(result.at("TeamMaxUsers")));
				contest->setOrganizerId(std::stoi(result.at("OrganizerID")));


                ServerMng::getInstance()->getChallMng()->addContest(contest);
                registerObject(conn, "contest:" + std::to_string(id));

                query = "SELECT ChallengeID FROM Challenges WHERE ContestID = '" + std::to_string(id) + "'";
                auto challResults = ServerMng::getInstance()->getDBController()->executeQuery(query);

                for (const auto& challResult : challResults)
                {
                    int challId = std::stoi(challResult.at("ChallengeID"));
                    loadChall(challId, contest, conn);
                }
            }
        }
        else
        {
            printError("[Loader] Contest not found: " + id );
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

                query = "SELECT ChallengeID FROM Challenges WHERE TabID = '" + std::to_string(id) + "'";
                auto challResults = ServerMng::getInstance()->getDBController()->executeQuery(query);

                for (const auto& challResult : challResults)
                {
                    int challId = std::stoi(challResult.at("ChallengeID"));
                    loadChall(challId, tab, conn);
                }
            }
        }
        else
        {
            printError("[Loader] Tab not found: " + id );
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
    printInfo("[Loader] Saved all objects for connection.");
}

void Loader::unloadObject(Connection* conn, const std::string& objId)
{
    size_t pos = objId.find(':');
    if (pos != std::string::npos)
    {
        std::string type = objId.substr(0, pos);
        std::string id = objId.substr(pos + 1);
        int numId = std::stoi(id);

        if (type == "user")
        {
            cleanupUser(numId, conn);
        }
        else if (type == "team")
        {
            cleanupTeam(numId, conn);
        }
        else if (type == "contest")
        {
            cleanupContest(numId, conn);
        }
        else if (type == "tab")
        {
            cleanupTab(numId, conn);
        }
        else if (type == "chall")
        {
            cleanupChall(numId, conn);
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
        printInfo("[Loader] Unloaded all objects for connection.");
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
    printInfo("[Loader] Registered object: " + objId + " for connection.");
}

bool Loader::isLoaded(Connection* conn, const std::string& objId) const
{
    if (objLoaded.count(conn) == 0) {
        return false;
    }
    return objLoaded.at(conn).find(objId) != objLoaded.at(conn).end();
}

// Specialized save functions
void Loader::saveUser(int userId)
{
    auto& users = ServerMng::getInstance()->getUsers();
    if (users.find(userId) != users.end() && users[userId].first != nullptr)
    {
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

        std::string checkQuery = "SELECT COUNT(*) as count FROM Users WHERE UserID = '" + std::to_string(userId) + "'";
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
                "' WHERE UserID = '" + std::to_string(userId) + "'";
            ServerMng::getInstance()->getDBController()->executeUpdate(query);
        }
        else {
            std::string query = "INSERT INTO Users (Username, PasswordHash, Email, Role) VALUES ('" + user->GetUsername() +
                "', '" + user->GetPassword() + "', '" + user->GetEmail() + "', '" + role + "')";
            ServerMng::getInstance()->getDBController()->executeUpdate(query);
        }
        printInfo("[Loader] Saved user: " + userId );
    }
}

void Loader::saveTeam(int teamId)
{
    auto& teams = ServerMng::getInstance()->getTeams();
    if (teams.find(teamId) != teams.end() && teams[teamId].first != nullptr) {
        Team* team = teams[teamId].first;

        std::string checkQuery = "SELECT COUNT(*) as count FROM Teams WHERE TeamID = '" + std::to_string(teamId) + "'";
        auto results = ServerMng::getInstance()->getDBController()->executeQuery(checkQuery);

        bool teamExists = false;
        if (!results.empty()) {
            teamExists = std::stoi(results[0]["count"]) > 0;
        }

        if (teamExists) {
            std::string query = "UPDATE Teams SET TeamName = '" + team->GetName() +
                "', LeaderID = '" + std::to_string(team->GetLeader()) +
                "', ContestID = '" + std::to_string(team->GetContestId()) +
                "' WHERE TeamID = '" + std::to_string(teamId) + "'";
            ServerMng::getInstance()->getDBController()->executeUpdate(query);
        }
        else {
            std::string query = "INSERT INTO Teams (TeamID, TeamName, LeaderID, ContestID) VALUES ('" +
                std::to_string(teamId) + "', '" + team->GetName() + "', '" + std::to_string(team->GetLeader()) +
                "', '" + std::to_string(team->GetContestId()) + "')";
            ServerMng::getInstance()->getDBController()->executeUpdate(query);

            // Also save team members relationship
            auto members = team->GetMembers();
            for (auto* member : members) {
                std::string relationQuery = "INSERT INTO UserTeams (UserID, TeamID) VALUES ('" +
                    std::to_string(member->GetId()) + "', '" + std::to_string(teamId) + "')";
                ServerMng::getInstance()->getDBController()->executeUpdate(relationQuery);
            }
        }
        printInfo("[Loader] Saved team: " + teamId );
    }
}

void Loader::saveChall(int challId)
{
    Chall* chall = nullptr;
    int contestId = 0;
    int tabId = 0;

    auto* challMng = ServerMng::getInstance()->getChallMng();

    // Search in contests
    for (const auto& contestPair : challMng->getAllContests()) {
        auto* contest = contestPair.second;
        auto challenges = contest->getChallenges();
        if (challenges.find(challId) != challenges.end()) {
            chall = challenges[challId];
            contestId = contest->getId();
            break;
        }
    }

    // If not in contests, search in tabs
    if (!chall) {
        for (const auto& tabPair : challMng->getAllTabs()) {
            auto* tab = tabPair.second;
            auto challenges = tab->getChallenges();
            if (challenges.find(challId) != challenges.end()) {
                chall = challenges[challId];
                tabId = tab->getId();
                break;
            }
        }
    }

    if (!chall) return;

    auto* db = ServerMng::getInstance()->getDBController();

    // Check if challenge exists
    std::string checkQuery = "SELECT * FROM Challenges WHERE ChallengeID = '" + std::to_string(challId) + "'";
    auto results = db->executeQuery(checkQuery);

    bool challExists = !results.empty();

    std::string name = chall->getName();
    std::string description = chall->getDescription();
    std::string flag = chall->getFlag();
    std::string tags = chall->getTags();
    std::string filesPath = chall->getFilesPath();

    if (challExists) {
        std::string query = "UPDATE Challenges SET "
            "Name = '" + name + "', "
            "Description = '" + description + "', "
            "AuthorID = '" + std::to_string(chall->getAuthor()) + "', ";

        if (contestId != 0) {
            query += "ContestID = '" + std::to_string(contestId) + "', ";
        }
        if (tabId != 0) {
            query += "TabID = '" + std::to_string(tabId) + "', ";
        }

        query +=
            "Difficulty = '" + chall->getDiffStr() + "', "
            "Score = '" + std::to_string(chall->getPoints()) + "', "
            "Flag = '" + flag + "', "
            "Tags = '" + tags + "', "
			"FilesPath = '" + filesPath + "' "
            "WHERE ChallengeID = '" + std::to_string(challId) + "'";

        db->executeUpdate(query);
    }
    else {
        std::string columns = "ChallengeID, Name, Description, AuthorID, ";
        std::string values = "'" + std::to_string(challId) + "', '" + name + "', '" + description + "', '" + std::to_string(chall->getAuthor()) + "', ";

        if (contestId != 0) {
            columns += "ContestID, ";
            values += "'" + std::to_string(contestId) + "', ";
        }

        if (tabId != 0) {
            columns += "TabID, ";
            values += "'" + std::to_string(tabId) + "', ";
        }

        columns += "Difficulty, Score, Flag, Tags, FilesPath";
        values += "'" + chall->getDiffStr() + "', '" +
            std::to_string(chall->getPoints()) + "', '" +
            flag + "', '" +
            tags + "', '" +
            filesPath;

        std::string query = "INSERT INTO Challenges (" + columns + ") VALUES (" + values + ")";
        db->executeUpdate(query);

    }

    printInfo("[Loader] Saved challenge: " + challId );
}

void Loader::saveContest(int contestId)
{
    auto* contest = ServerMng::getInstance()->getContest(contestId);
    if (contest) {
        std::string checkQuery = "SELECT COUNT(*) as count FROM Contests WHERE ContestID = '" + std::to_string(contestId) + "'";
        auto results = ServerMng::getInstance()->getDBController()->executeQuery(checkQuery);

        bool contestExists = false;
        if (!results.empty()) {
            contestExists = std::stoi(results[0]["count"]) > 0;
        }
		//contest->getStartTime() -- time_t / The update command expects an input like yyyy-mm-dd hh:mm:ss.000
        auto formatTime = [](time_t time) {
            using namespace std::chrono;
            system_clock::time_point tp = floor<seconds>(system_clock::from_time_t(time + 7200));
            std::string out = std::format("{:%Y-%m-%d %H:%M:%S}", tp);
			out = out.substr(0, out.find('.'));
			return out;
        };

        std::string start = formatTime(contest->getStartTime());

		std::string end = formatTime(contest->getEndTime());


        if (contestExists) {
            std::string query = "UPDATE Contests SET Name = '" + contest->getName() +
                "', Description = '" + contest->getDescription() +
                "', StartDate = '" + start +
                "', EndDate = '" + end +
                "', TeamMaxUsers = '" + std::to_string(contest->getMaxTeamUsers()) +
                "', OrganizerID = '" + std::to_string(contest->getOrganizerId()) +
                "' WHERE ContestID = '" + std::to_string(contestId) + "'";
            ServerMng::getInstance()->getDBController()->executeUpdate(query);
        }
        else {
            std::string query = "INSERT INTO Contests (ContestID, Name, Description, StartDate, EndDate, TeamMaxUsers, OrganizerID) VALUES ('" +
                std::to_string(contestId) + "', '" +
                contest->getName() + "', '" +
                contest->getDescription() + "', '" +
                start + "', '" +
                end + "', '" +
                std::to_string(contest->getMaxTeamUsers()) + "', '" +
                std::to_string(contest->getOrganizerId()) +"')";
            ServerMng::getInstance()->getDBController()->executeUpdate(query);
        }

        auto challenges = contest->getChallenges();
        for (auto& challPair : challenges) {
            saveChall(challPair.first);
        }

        printInfo("[Loader] Saved contest: " + contestId );
    }
}

void Loader::saveTab(int tabId)
{
    auto* tab = ServerMng::getInstance()->getTab(tabId);
    if (tab) {
        std::string checkQuery = "SELECT COUNT(*) as count FROM Tabs WHERE TabID = '" + std::to_string(tabId) + "'";
        auto results = ServerMng::getInstance()->getDBController()->executeQuery(checkQuery);

        bool tabExists = false;
        if (!results.empty()) {
            tabExists = std::stoi(results[0]["count"]) > 0;
        }

        if (tabExists) {
            std::string query = "UPDATE Tabs SET Name = '" + tab->getName() +
                "' WHERE TabID = '" + std::to_string(tabId) + "'";
            ServerMng::getInstance()->getDBController()->executeUpdate(query);
        }
        else {
            std::string query = "INSERT INTO Tabs (TabID, Name) VALUES ('" +
                std::to_string(tabId) + "', '" + tab->getName() + "')";
            ServerMng::getInstance()->getDBController()->executeUpdate(query);
        }

        auto challenges = tab->getChallenges();
        for (auto& challPair : challenges) {
            saveChall(challPair.first);
        }

        printInfo("[Loader] Saved tab: " + tabId );
    }
}

void Loader::saveObject(const std::string& objId)
{
    size_t pos = objId.find(':');
    if (pos != std::string::npos) {
        std::string type = objId.substr(0, pos);
        std::string id = objId.substr(pos + 1);
        int numId = std::stoi(id);

        if (type == "user") {
            saveUser(numId);
        }
        else if (type == "team") {
            saveTeam(numId);
        }
        else if (type == "chall") {
            saveChall(numId);
        }
        else if (type == "contest") {
            saveContest(numId);
        }
        else if (type == "tab") {
            saveTab(numId);
        }
    }
}

// ---------------------- Cleanup functions with cascading logic ----------------------

void Loader::cleanupUser(int userId, Connection* conn)
{
    auto& users = ServerMng::getInstance()->getUsers();
    auto userIt = users.find(userId);
    if (userIt != users.end())
    {
        auto& connections = userIt->second.second;
        connections.remove_if([&](Connection* connection) { return connection == conn || !connection->isConnected(); });

        if (connections.empty())
        {
            User* user = userIt->second.first;

            auto& teams = ServerMng::getInstance()->getTeams();
            for (auto teamIt = teams.begin(); teamIt != teams.end(); )
            {
                Team* team = teamIt->second.first;
                if (team && team->GetMember(userId)!=nullptr)
                {
                    team->RemoveMember(userId);
                    if (team->GetMembers().empty() || team->GetLeader() == userId)
                    {
                        saveTeam(teamIt->first);
                        cleanupTeam(teamIt->first, nullptr); 
                        teamIt = teams.begin();
                    }
                    else
                    {
                        ++teamIt;
                    }
                }
                else
                {
                    ++teamIt;
                }
            }

            auto* challMng = ServerMng::getInstance()->getChallMng();
            for (auto& contestPair : challMng->getAllContests())
            {
                auto challenges = contestPair.second->getChallenges();
                for (auto& challPair : challenges)
                {
                    if (challPair.second->getAuthor() == userId)
                    {
                        challPair.second->setAuthor(0); 
                        saveChall(challPair.first);
                    }
                }
            }
            for (auto& tabPair : challMng->getAllTabs())
            {
                auto challenges = tabPair.second->getChallenges();
                for (auto& challPair : challenges)
                {
                    if (challPair.second->getAuthor() == userId)
                    {
                        challPair.second->setAuthor(0); 
                        saveChall(challPair.first);
                    }
                }
            }

            delete user;
            users.erase(userIt);
            printInfo("[Loader] Cleaned up user: " + userId );
        }
    }
}

void Loader::cleanupTeam(int teamId, Connection* conn)
{
    auto& teams = ServerMng::getInstance()->getTeams();
    auto teamIt = teams.find(teamId);
    if (teamIt != teams.end())
    {
        auto& connections = teamIt->second.second;
        connections.remove_if([&](Connection* connection) { return connection == conn || !connection->isConnected(); });

        if (connections.empty())
        {
            Team* team = teamIt->second.first;

            // Cascade: Remove team from its contest
            int contestId = team->GetContestId();
            auto* contest = ServerMng::getInstance()->getContest(contestId);
            if (contest)
            {
                contest->removeTeam(teamId);
                saveContest(contestId);
            }

            for (auto* member : team->GetMembers())
            {
                auto userIt = ServerMng::getInstance()->getUsers().find(member->GetId());
                if (userIt != ServerMng::getInstance()->getUsers().end())
                {
                    // Optionally notify user or update user state
                    // For now, just ensure consistency
                }
            }

            delete team;
            teams.erase(teamIt);
            printInfo("[Loader] Cleaned up team: " + teamId );
        }
    }
}

void Loader::cleanupChall(int challId, Connection* conn)
{
    auto* challMng = ServerMng::getInstance()->getChallMng();
    bool deleted = false;

    // Check contests
    for (auto& contestPair : challMng->getAllContests())
    {
        auto* contest = contestPair.second;
        auto challenges = contest->getChallenges();
        if (challenges.find(challId) != challenges.end())
        {
            bool inTab = false;
            for (auto& tabPair : challMng->getAllTabs())
            {
                if (tabPair.second->getChallenges().find(challId) != tabPair.second->getChallenges().end())
                {
                    inTab = true;
                    break;
                }
            }

            if (!inTab)
            {
                delete challenges[challId]; 
                deleted = true;
            }
            contest->removeChallenge(challId);
            printInfo("[Loader] Removed challenge " + std::to_string(challId) + " from contest " + std::to_string(contestPair.first));
        }
    }

    for (auto& tabPair : challMng->getAllTabs())
    {
        auto* tab = tabPair.second;
        auto challenges = tab->getChallenges();
        if (challenges.find(challId) != challenges.end())
        {
            bool inContest = false;
            for (auto& contestPair : challMng->getAllContests())
            {
                if (contestPair.second->getChallenges().find(challId) != contestPair.second->getChallenges().end())
                {
                    inContest = true;
                    break;
                }
            }

            if (!inContest && !deleted)
            {
                delete challenges[challId];
            }
            tab->removeChallenge(challId);
            printInfo("[Loader] Removed challenge " + std::to_string(challId)+ " from tab " + std::to_string(tabPair.first));
        }
    }
}

void Loader::cleanupContest(int contestId, Connection* conn)
{
    auto* contest = ServerMng::getInstance()->getContest(contestId);
    if (contest)
    {
        auto challenges = contest->getChallenges();
        std::vector<int> challIds;

        for (auto& challPair : challenges)
        {
            challIds.push_back(challPair.first);
        }

        for (int challId : challIds)
        {
            cleanupChall(challId, conn);
        }

        auto& teams = ServerMng::getInstance()->getTeams();
        for (auto teamIt = teams.begin(); teamIt != teams.end(); )
        {
            if (teamIt->second.first->GetContestId() == contestId)
            {
                saveTeam(teamIt->first); 
                cleanupTeam(teamIt->first, nullptr);
                teamIt = teams.begin(); 
            }
            else
            {
                ++teamIt;
            }
        }

        ServerMng::getInstance()->getChallMng()->removeContest(contestId);
        printInfo("[Loader] Cleaned up contest: " + contestId );
    }
}

void Loader::cleanupTab(int tabId, Connection* conn)
{
    auto* tab = ServerMng::getInstance()->getTab(tabId);
    if (tab)
    {
        auto challenges = tab->getChallenges();
        std::vector<int> challIds;

        for (auto& challPair : challenges)
        {
            challIds.push_back(challPair.first);
        }

        for (int challId : challIds)
        {
            cleanupChall(challId, conn);
        }

        ServerMng::getInstance()->getChallMng()->removeTab(tabId);
        printInfo("[Loader] Cleaned up tab: " + tabId );
    }
}