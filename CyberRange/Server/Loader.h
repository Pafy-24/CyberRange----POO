#pragma once
#include <string>
#include <map>
#include "Controller.h"
#include "Connection.h"

class Loader {
private: 
	static Loader* instance;
    Loader()=default;

	Loader(const Loader&) = delete;
	Loader& operator=(const Loader&) = delete;
	Loader(Loader&&) = delete;
	Loader& operator=(Loader&&) = delete;

public:
    static Loader* getInstance();
    void loadUsers();
    void loadChalls();
    void loadTeams();
	void loadContests();
    void loadAll();
    void saveUnload();
    Controller* getController(std::string name);
    void registerObject(Connection* conn, std::string objId);
    void attachControllers(std::map<std::string, Controller*> controllerMAP);
    bool isLoaded(std::string objId);
private:
    std::map<std::string, Controller*> controllerMap;
    std::map<Connection*, std::string> objLoaded;
    std::string basePath;

};
