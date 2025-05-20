#include "pch.h"
#include "ClientMng.h"
#include "ConnsFactory.h"
#include "ChallClientController.h"
#include "ContestClientController.h"
#include "AuthController.h"
#include "TabClientController.h"
#include <iostream>
#include <thread>
#include <QTimer>
#include <json.hpp>

using json = nlohmann::json;

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
    : port(port), serverAddress(address), loader(new Loader()), serverConn(nullptr), connected(false)
{
    attachController("ChallClientController", new ChallClientController());
    attachController("ContestClientController", new ContestClientController());
    attachController("TabClientController", new TabClientController());
    attachController("AuthController", new AuthController());

}

ClientMng::~ClientMng()
{
    stop();
    std::lock_guard<std::mutex> lock(instanceMutex);
    instance = nullptr;
}

bool ClientMng::start()
{
    if (connected && serverConn && serverConn->isConnected())
    {
        std::cout << "Client already connected.\n";
        return true;
    }

    if (serverConn) {
        serverConn->disconnect();
        delete serverConn;
        serverConn = nullptr;
    }

    try {
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
    catch (const std::exception& e) {
        std::cout << "Exception during connection: " << e.what() << "\n";
        return false;
    }
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

    {
        std::lock_guard<std::mutex> lock(controllerMutex);
        for (auto& pair : controllers)
        {
            delete pair.second;
        }
        controllers.clear();
    }

    delete loader;
    loader = nullptr;

    connected = false;
    std::cout << "Client connection closed.\n";
}

void ClientMng::sendRequest(const std::string& data)
{
    if (!connected || !serverConn)
    {
        std::cout << "Cannot send request: not connected to server. Attempting to reconnect...\n";
        if (!start()) {
            std::cout << "Reconnection failed.\n";
            throw std::runtime_error("Failed to connect to server");
        }
    }

    if (!serverConn->send(data)) {
        std::cout << "Failed to send data.\n";
        throw std::runtime_error("Failed to send data to server");
    }
}

void ClientMng::receiveResponse()
{
    if (!connected || !serverConn)
    {
        std::cout << "Cannot receive response: not connected to server.\n";
        throw std::runtime_error("Not connected to server");
    }

    std::string response = serverConn->receive();
    if (response.empty()) {
        std::cout << "Received empty response from server.\n";
        return;
    }

    try {
        auto jsonResponse = json::parse(response);
        if (!jsonResponse.contains("controller")) {
            std::cout << "Invalid response format: missing controller field.\n";
            return;
        }

        std::string ctrlName = jsonResponse["controller"];
        std::lock_guard<std::mutex> lock(controllerMutex);
        auto it = controllers.find(ctrlName);
        if (it == controllers.end()) {
            std::cout << "No controller found for: " << ctrlName << "\n";
            return;
        }

        it->second->handleServerResponse(response);
    }
    catch (const std::exception& e) {
        std::cout << "Error processing response: " << e.what() << "\n";
        std::cout << "Response was: " << response << "\n";
    }
}

void ClientMng::attachController(const std::string& name, Controller* ctrl)
{
    if (ctrl)
    {
        std::lock_guard<std::mutex> lock(controllerMutex);
        auto it = controllers.find(name);
        if (it != controllers.end())
        {
            delete it->second;  
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
    auto it = controllers.find(name);
    return it != controllers.end() ? it->second : nullptr;
}

Connection* ClientMng::getConnection()
{
    return serverConn;
}

bool ClientMng::isConnected() const
{
    return connected && serverConn && serverConn->isConnected();
}