#pragma once
#include <string>
#include <map>
#include <set>
#include "Controller.h"
#include "Connection.h"

class Loader {
private:
    std::map<Connection*, std::set<std::string>> objLoaded;
    std::string basePath;

public:
    void loadUser(int id, Connection* conn);
    void loadUserByUsername(std::string username, Connection* conn);
    void loadUserByEmail(std::string email, Connection* conn);

    void loadChall(int id, Connection* conn);
    void loadTeam(int id, Connection* conn);
    void loadContest(int id, Connection* conn);
    // void loadTab(int id, Connection* conn); TO DO

    void save(Connection* conn);
    void unload(Connection* conn);
    void saveUnload(Connection* conn);

    void registerObject(Connection* conn, std::string objId);
    // TO DO bool isLoaded(std::string objId) const;

private:
    // TO DO void saveObject(const std::string& objId);
};