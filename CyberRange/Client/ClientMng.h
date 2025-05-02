#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include "Connection.h"
#include "Loader.h"
#include "Controller.h"

class ClientMng { //singleton clientmng
private:
	static ClientMng* instance;
    
    // Mutex to ensure thread safety
    static std::mutex mtx;

    ClientMng(std::string address, int port); // constructor

	ClientMng(const ClientMng&); // copy constructor
	ClientMng& operator=(const ClientMng&); // assignment operator

	ClientMng(ClientMng&&); // move constructor
	ClientMng& operator=(ClientMng&&); // move assignment operator
	~ClientMng(); // destructor

public:
	static ClientMng* getInstance(); 
	void setValues(std::string address, int port); 
    bool connect();
    void disconnect();
    void sendRequest(const std::string& requestData);
    std::string receiveResponse();  // sau poate chiar bool + out param
    void registerController(Controller* ctrl);
    Controller* getController(std::string name);


private:
    Connection* serverConn;
    Loader* loader;
    std::vector<Controller*> controllers;
    bool connected;
    std::string serverAddress;
    int serverPort;
};