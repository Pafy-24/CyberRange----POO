#pragma once
#include <string>
#include <map>
#include <set>
#include "Controller.h"
#include "Connection.h"
#include "Tab.h"
#include "Contest.h"


class Loader {
private:
    std::map<Connection*, std::set<std::string>> objLoaded;
    std::string basePath;

public:
    void loadUser(int id, Connection* conn);
    void loadUserByUsername(const std::string& username, Connection* conn);
    void loadUserByEmail(const std::string& email, Connection* conn);
   
    void loadTeam(int id, Connection* conn);

    void loadChall(int id, Contest* mng, Connection* conn);

    void loadChall(int id, Tab* mng, Connection* conn);

    void loadContest(int id, Connection* conn);
    void loadTab(int id, Connection* conn); 

    void save(Connection* conn);
    void unload(Connection* conn);
    void saveUnload(Connection* conn);

    void registerObject(Connection* conn, const std::string& objId);

    // TO DO bool isLoaded(std::string objId) const;

private:
    // TO DO void saveObject(const std::string& objId);
};