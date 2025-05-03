#include "Loader.h"
#include "UserController.h"
#include "TeamController.h"
#include "ChallController.h"
#include "ContestController.h"

Loader* Loader::instance = nullptr;
Loader* Loader::getInstance()
{
    if (!instance) 
    {
        instance = new Loader();
    }
    return instance;
}

void Loader::loadUsers()
{
    auto* userCtrl = dynamic_cast<UserController*>(getController("UserController"));
    if (!userCtrl) 
    {
        std::cout << "[Loader] UserController not found\n";
        return;
    }

    DBController* db = userCtrl->getDB();
    if (!db)
    {
        std::cout << "[Loader] DBController not found in UserController\n";
        return;
    }

    auto results = db->executeQuery("SELECT UserID FROM Users");
    int count = 0;

    for (const auto& row : results) 
    {
        if (row.count("UserID")) 
        {
            userCtrl->loadUser(row.at("userId"));
            count++;
        }
    }

    std::cout << "[Loader] Loaded " << count << " users from DB.\n";
}

void Loader::loadChalls()
{
    auto* challCtrl = dynamic_cast<ChallController*>(getController("ChallController"));
    if (!challCtrl) 
    {
        std::cout << "[Loader] ChallController not found\n";
        return;
    }

    DBController* db = challCtrl->getDB();
    if (!db) 
    {
        std::cout << "[Loader] DBController not found in ChallController\n";
        return;
    }

    std::vector<std::map<std::string, std::string>> results = db->executeQuery("SELECT ChallengeID FROM Challenges");

    int count = 0;
    for (const auto& row : results) 
    {
        if (row.count("challId")) 
        {
            challCtrl->loadChallenge(row.at("challId"));
            count++;
        }
    }

    std::cout << "[Loader] Loaded " << count << " challenges from DB.\n";
}

void Loader::loadTeams()
{
    auto* teamCtrl = dynamic_cast<TeamController*>(getController("TeamController"));
    if (!teamCtrl) 
    {
        std::cout << "[Loader] TeamController not found\n";
        return;
    }

    DBController* db = teamCtrl->getDB();
    if (!db) 
    {
        std::cout << "[Loader] DBController not found in TeamController\n";
        return;
    }

    auto results = db->executeQuery("SELECT teamId FROM Teams");
    int count = 0;

    for (const auto& row : results) 
    {
        if (row.count("teamId")) 
        {
            teamCtrl->loadTeam(row.at("teamId"));
            count++;
        }
    }

    std::cout << "[Loader] Loaded " << count << " teams from DB.\n";
}

void Loader::loadContests()
{
	auto* contestCtrl = dynamic_cast<ContestController*>(getController("ContestController"));
	if (!contestCtrl)
	{
		std::cout << "[Loader] ContestController not found\n";
		return;
	}
	DBController* db = contestCtrl->getDB();
	if (!db)
	{
		std::cout << "[Loader] DBController not found in ContestController\n";
		return;
	}
	auto results = db->executeQuery("SELECT ContestID FROM Contests");
	int count = 0;
	for (const auto& row : results)
	{
		if (row.count("contestId"))
		{
			contestCtrl->loadContest(row.at("contestId"));
			count++;
		}
	}
	std::cout << "[Loader] Loaded " << count << " contests from DB.\n";
}

void Loader::loadAll()
{
    std::cout << "[Loader] Loading all entities..." << std::endl;

    loadUsers();
    loadTeams();
	loadChalls();
	loadContests();

    std::cout << "[Loader] All entities loaded." << std::endl;
}

void Loader::saveUnload()
{
    std::cout << "[Loader] Saving and unloading all entities..." << std::endl;

    auto* userCtrl = dynamic_cast<UserController*>(getController("UserController"));
    auto* teamCtrl = dynamic_cast<TeamController*>(getController("TeamController"));
    auto* challCtrl = dynamic_cast<ChallController*>(getController("ChallController"));

    if (userCtrl) 
    {
        for (const auto& pair : objLoaded) 
        {
            if (pair.second == "UserController") 
            {
                //userCtrl->unloadUser(pair.first->getId());
            }
        }
    }

    if (teamCtrl) 
    {
        for (const auto& pair : objLoaded) 
        {
            if (pair.second == "TeamController") 
            {
                //teamCtrl->unloadTeam(pair.first->getId());
            }
        }
    }

    if (challCtrl) 
    {
        for (const auto& pair : objLoaded) 
        {
            if (pair.second == "ChallController") 
            {
                //challCtrl->unloadChallenge(pair.first->getId());
            }
        }
    }

    std::cout<<"[Loader] All entities unloaded." << std::endl;
}

Controller* Loader::getController(std::string name)
{
    auto it = controllerMap.find(name);
    if (it != controllerMap.end()) 
    {
        return it->second;
    }
    std::cout << "[Loader] Controller not found: " << name << std::endl;
    return nullptr;
}

void Loader::registerObject(Connection* conn, std::string objId)
{
    objLoaded[conn] = objId;
    std::cout << "[Loader] Registered object: " << objId << std::endl;
}

void Loader::attachControllers(std::map<std::string, Controller*> controllerMAP)
{
	this->controllerMap = controllerMAP;
}

bool Loader::isLoaded(std::string objId)
{
    for (const auto& pair : objLoaded) 
    {
        if (pair.second == objId) 
        {
            return true;
        }
    }
    return false;
}
