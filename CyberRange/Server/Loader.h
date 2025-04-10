#pragma once
#include <string>
#include <map>
#include "Controller.h"
#include "Connection.h"
#include "JSONEnc.h"
#include "JSONDec.h"

class Loader {
private:
    std::map<std::string, Controller*> controllerMap;
    std::map<Connection*, std::string> objLoaded;
    std::string basePath;
    JSONEnc encoder;
    JSONDec decoder;

public:
    Loader(std::string path);
    void load();
    void save();
    void loadAll();
    void saveUnload();
    Controller* getController(std::string name);
    void registerObject(Connection* conn, std::string objId);
    bool isLoaded(std::string objId);
};
