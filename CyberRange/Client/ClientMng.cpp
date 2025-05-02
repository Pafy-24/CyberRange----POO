#include "pch.h"
#include "ClientMng.h"
#include "ConnsFactory.h"
#include <iostream>
#include <thread>

ClientMng* ClientMng::instance = nullptr;
std::mutex ClientMng::instanceMutex;

ClientMng* ClientMng::getInstance(int port, const std::string& address) 
{
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (instance == nullptr) 
    {
        instance = new ClientMng(port, address);
    }
    return instance;
}

ClientMng::ClientMng(int port, const std::string& address)
    : port(port), serverAddress(address), loader(new Loader()), serverConn(nullptr), connected(false) {}

ClientMng::~ClientMng() 
{
    stop();
    std::lock_guard<std::mutex> lock(instanceMutex);
    instance = nullptr;
}

bool ClientMng::start() 
{
    if (connected) 
    {
        std::cout << "Client already connected.\n";
        return false;
    }

    auto conn = ConnsFactory::createConnection(ConnectionType::TCP, serverAddress, port);
    if (!conn) 
    {
        std::cout << "Failed to create connection object.\n";
        return false;
    }

    if (!conn->connect()) 
    {
        std::cout << "Failed to connect to server at " << serverAddress << ":" << port << "\n";
        return false;
    }
    serverConn = conn.release();

    connected = true;
    std::cout << "Connected to server at " << serverAddress << ":" << port << "\n";
    return connected;
}

void ClientMng::stop() 
{
    if (!connected) return;

    if (serverConn) 
    {
        serverConn->disconnect();
        delete serverConn;
        serverConn = nullptr;
    }

    for (auto& pair : controllers) 
    {
        delete pair.second;
    }
    controllers.clear();

    delete loader;
    loader = nullptr;

    connected = false;
    std::cout << "Client connection closed.\n";
}

void ClientMng::sendRequest(const std::string& data) 
{
    if (!connected || !serverConn) 
    {
        std::cout << "Cannot send request: not connected to server.\n";
        return;
    }

    serverConn->send(data);
}

void ClientMng::receiveResponse() 
{
    if (!connected || !serverConn) 
    {
        std::cout << "Cannot receive response: not connected to server.\n";
        return;
    }

    std::string response = serverConn->receive();
    if (response.empty()) return;

    std::lock_guard<std::mutex> lock(controllerMutex);
    for (const auto& pair : controllers) 
    {
        Controller* ctrl = pair.second;
        if (ctrl->validateResponse(response)) 
        {
            ctrl->handleServerResponse(response);
            return;
        }
    }
    std::cout << "No controller handled the response: " << response << "\n";
}

void ClientMng::attachController(const std::string& name, Controller* ctrl) 
{
    if (ctrl) 
    {
        std::lock_guard<std::mutex> lock(controllerMutex);
        auto it = controllers.find(name);
        if (it != controllers.end()) 
        {
            delete it->second;  // evitam un memory leak
        }
        controllers[name] = ctrl;
        std::cout << "Attached client controller: " << name << "\n";
    }
}

void ClientMng::removeController(const std::string& name) 
{
    std::lock_guard<std::mutex> lock(controllerMutex);
    auto it = controllers.find(name);
    if (it != controllers.end()) 
    {
        delete it->second;
        controllers.erase(it);
        std::cout << "Removed client controller: " << name << "\n";
    }
}

Controller* ClientMng::getController(const std::string& name) 
{
    std::lock_guard<std::mutex> lock(controllerMutex);
    auto it = controllers.find(name);
    return it != controllers.end() ? it->second : nullptr;
}

Connection* ClientMng::getConnection()
{
	return serverConn;
}

bool ClientMng::isConnected() const 
{
    return connected;
}