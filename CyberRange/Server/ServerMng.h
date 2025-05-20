#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <atomic>
#include "Observable.h"
#include "Controller.h"
#include "DBController.h"
#include "Connection.h"
#include "User.h"
#include "Team.h"
#include "Loader.h"
#include "ChallMng.h"

class ServerMng : public Observable {
private:


    static ServerMng* instance;

    std::atomic<bool> isRunning;


    std::vector<Connection*> servers;
    std::vector<Connection*> connections;
    std::map<std::string, Controller*> controllers;
    DBController* dbController;
    ChallMng challMng;

    std::map<int, std::pair<Team*, std::list<Connection*>>> teams;
    std::map<int, std::pair<User*, std::list<Connection*>>> users;

    std::vector<std::string> tokens;
    std::string SecretKey;
    Loader* loader;
    int port;
    std::string serverAddress;

    ServerMng(int port, std::string address);
    void runTCPServer(int port, bool useTLS);
    void runUDPServer(int port);
    void runTransferServer(int port);

    ServerMng(const ServerMng&) = delete;
    ServerMng& operator=(const ServerMng&) = delete;

public:
    static ServerMng* getInstance(int port = 1337, const std::string& address = "0.0.0.0");
    ~ServerMng();

    void start();
    void stop();
    void addConnection(Connection* conn);
    void removeConnection(Connection* conn);
    void attachController(std::string name, Controller* ctrl);
    void removeController(std::string name);
    Connection* getConnection(int id);
    Controller* getController(const std::string& name);
    ChallMng* getChallMng() { return &challMng; }
    Contest* getContest(int mngID);
    Tab* getTab(int mngID);
    void addToken(const std::string& token);
    void removeToken(const std::string& token);

    std::string getSecretKey() const { return SecretKey; }

    const std::vector<std::string>& getTokens() const { return tokens; }

    DBController* getDBController() { return dbController; }
    Loader* getLoader() { return loader; }
    std::map<int, std::pair<Team*, std::list<Connection*>>>& getTeams() { return teams; }
    std::map<int, std::pair<User*, std::list<Connection*>>>& getUsers() { return users; }

    void pushUser(User* user, Connection* conn);
    void pushTeam(Team* team, Connection* conn);

    User* findUser(const std::string& usrName_email);

};


/*
    inline void printMessage(const std::string& message) {
        if (ServerMng::getInstance(0, "")) {
            ServerMng::getInstance(0, "")->printLog(message);
        }
        else {
            std::cout << message << std::endl;
        }
    }*/