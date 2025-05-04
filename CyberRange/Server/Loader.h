#pragma once
#include <string>
#include <map>
#include "Controller.h"
#include "Connection.h"

class Loader {
private:

    std::map<Connection*, std::string> objLoaded;
    std::string basePath;

public:

    void loadUser(int id, Connection* conn);
    void loadUser(std::string username, Connection* conn);
    void loadUser(std::string email, Connection* conn);

	void loadChall(int id, Connection* conn);
	void loadTeam(int id, Connection* conn);
	void loadContest(int id, Connection* conn);
	void loadTab(int id, Connection* conn);


    void save(Connection* conn);
	void unload(Connection* conn);
    void saveUnload(Connection* conn);
    void registerObject(Connection* conn, std::string objId);
    bool isLoaded(std::string objId);

};
