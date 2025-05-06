#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <mutex>
#include "Connection.h"
#include "Controller.h"
#include "Loader.h"

class ClientMng 
{
private:
    static ClientMng* instance;
    static std::mutex instanceMutex;

    std::mutex controllerMutex;
    std::map<std::string, Controller*> controllers;
    std::string AuthToken;
    Loader* loader;
    Connection* serverConn;
	ChallMng challMng;
    bool connected;
    int port;
    std::string serverAddress;

    ClientMng(int port, const std::string& address);

public:
    static ClientMng* getInstance(int port = 1337, const std::string& address = "127.0.0.1");
    ~ClientMng();

    ClientMng(const ClientMng&) = delete;
    ClientMng& operator=(const ClientMng&) = delete;

    bool start();        // init conexiune
    void stop();         // inchide conexiunea

    void sendRequest(const std::string& data);
    void receiveResponse();  // va delega la controllerul potrivit

    void attachController(const std::string& name, Controller* ctrl);
    void removeController(const std::string& name);

    Controller* getController(const std::string& name);
    Connection* getConnection();

    bool isConnected() const;

	void setAuthToken(const std::string& token) { AuthToken = token; }
	std::string getAuthToken() const { return AuthToken; }

};