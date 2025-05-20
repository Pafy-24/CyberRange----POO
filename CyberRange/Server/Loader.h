#pragma once
#include <string>
#include <map>
#include <set>
#include "Observable.h"
#include "Controller.h"
#include "Connection.h"
#include "Tab.h"
#include "Contest.h"


class Loader: public Observable{
private:
    std::map<Connection*, std::set<std::string>> objLoaded;
    std::string basePath;

public:
    void loadUser(int id, Connection* conn);
    int loadUserByUsername(const std::string& username, Connection* conn);
    int loadUserByEmail(const std::string& email, Connection* conn);

    void loadTeam(int id, Connection* conn);

    void loadChall(int id, Contest* mng, Connection* conn);

    void loadChall(int id, Tab* mng, Connection* conn);

    void loadContest(int id, Connection* conn);
    void loadTab(int id, Connection* conn);

    void save(Connection* conn);
	void unloadObject(Connection* conn, const std::string& objId);
    void unload(Connection* conn);
    void saveUnload(Connection* conn);

    void registerObject(Connection* conn, const std::string& objId);

    bool isLoaded(Connection* conn, const std::string& objId) const;

    void saveUser(int userId);

    void saveTeam(int teamId);

    void saveChall(int challId);

    void saveContest(int contestId);

    void saveTab(int tabId);

private:
    void saveObject(const std::string& objId);
    void cleanupUser(int userId, Connection* conn);
    void cleanupTeam(int teamId, Connection* conn);
    void cleanupChall(int challId, Connection* conn);
    void cleanupContest(int contestId, Connection* conn);
    void cleanupTab(int tabId, Connection* conn);
};

Controller* getController(const std::string& name);
