#include "Loader.h"
#include "UserController.h"
#include "TeamController.h"
#include "ChallController.h"
#include "ContestController.h"
#include "ServerMng.h"
#include <iostream>

Controller* getController(const std::string& name) 
{
    return ServerMng::getInstance()->getController(name);
}

// ---------------------- Load methods ----------------------

void Loader::loadUser(int id, Connection* conn) 
{
    auto* ctrl = dynamic_cast<UserController*>(getController("UserController"));
    if (ctrl) 
        ctrl->loadUser(std::to_string(id));
    registerObject(conn, "user:" + std::to_string(id));
}

void Loader::loadUserByUsername(std::string username, Connection* conn) 
{
    auto* ctrl = dynamic_cast<UserController*>(getController("UserController"));
    if (ctrl) 
        ctrl->loadUserByUsername(username);
    registerObject(conn, "user:" + username);
}

void Loader::loadUserByEmail(std::string email, Connection* conn) 
{
    auto* ctrl = dynamic_cast<UserController*>(getController("UserController"));
    if (ctrl) 
        ctrl->loadUserByEmail(email);
    registerObject(conn, "user:" + email);
}

void Loader::loadChall(int id, Connection* conn) 
{
    auto* ctrl = dynamic_cast<ChallController*>(getController("ChallController"));
    if (ctrl) 
        ctrl->loadChallenge(std::to_string(id));
    registerObject(conn, "chall:" + std::to_string(id));
}

void Loader::loadTeam(int id, Connection* conn)
{
    auto* ctrl = dynamic_cast<TeamController*>(getController("TeamController"));
    if (ctrl) 
        ctrl->loadTeam(std::to_string(id));
    registerObject(conn, "team:" + std::to_string(id));
}

void Loader::loadContest(int id, Connection* conn)
{
    auto* ctrl = dynamic_cast<ContestController*>(getController("ContestController"));
    if (ctrl)
        ctrl->loadContest(std::to_string(id));
    registerObject(conn, "contest:" + std::to_string(id));
}

//void Loader::loadTab(int id, Connection* conn)
//{
//    auto* ctrl = dynamic_cast<TabController*>(getController("TabController"));
//    if (ctrl) 
//        ctrl->loadTab(std::to_string(id));
//    registerObject(conn, "tab:" + std::to_string(id));
//}

// ---------------------- Save/Unload ----------------------

void Loader::save(Connection* conn) 
{
    if (objLoaded.count(conn)) 
    {
        for (const auto& objId : objLoaded[conn]) 
        {
            //saveObject(objId);
        }
    }
    std::cout << "[Loader] Saved all objects for connection.\n";
}

void Loader::unload(Connection* conn) 
{
    if (objLoaded.count(conn)) 
    {
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

void Loader::registerObject(Connection* conn, std::string objId) {
    objLoaded[conn].insert(objId);

    std::cout << "[Loader] Registered object: " << objId << " for connection.\n";
}