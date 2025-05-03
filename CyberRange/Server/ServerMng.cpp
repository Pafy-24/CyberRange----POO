#include "pch.h"
#include "ServerMng.h"
#include "CustomSerial.h"
#include "ConnsFactory.h"
#include "DBController.h"
#include "UserController.h"
#include "TeamController.h"
#include "ChallController.h"
#include <iostream>
#include <algorithm>
#include <thread>

ServerMng* ServerMng::instance = nullptr;
std::mutex ServerMng::instanceMutex;

ServerMng* ServerMng::getInstance(int port, const std::string& address) {
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (instance == nullptr) {
        instance = new ServerMng(port, address);
    }
    return instance;
}

ServerMng::ServerMng(int port, std::string address)
    : isRunning(false), port(port), serverAddress(address), loader(new Loader()) {
    // Initialize DBController
    DBController* dbCtrl = new DBController("sqlserver://administrator:StrongP@ssw0rd!@localhost:1433/CyberRangeDB");
    attachController("DBController", dbCtrl);

    // Initialize other controllers
    attachController("UserController", new UserController(dbCtrl));
    attachController("TeamController", new TeamController(dbCtrl));
    attachController("ChallController", new ChallController(dbCtrl));
    attachController("default", new Controller());
}

ServerMng::~ServerMng() {
    stop();
}

void ServerMng::start() {
    if (isRunning) {
        std::cout << "Server manager already running" << std::endl;
        return;
    }
    isRunning = true;

    // Load initial data
    loader->loadAll();

    runTCPServer(port, false);
    runDownloadServer(port + 3);

    std::cout << "Server manager started" << std::endl;
}

void ServerMng::stop() {
    if (!isRunning) {
        return;
    }
    isRunning = false;

    // Save and unload data
    loader->saveUnload();

    {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        for (auto* conn : connections) {
            conn->stopServer();
            delete conn;
        }
        connections.clear();
    }

    for (auto& ctrl : controllers) {
        delete ctrl.second;
    }
    controllers.clear();

    for (auto* chall : challMngs) {
        delete chall;
    }
    challMngs.clear();

    for (auto* user : users) {
        delete user;
    }
    users.clear();

    delete loader;
    loader = nullptr;

    std::lock_guard<std::mutex> lock(instanceMutex);
    instance = nullptr;

    std::cout << "Server manager stopped" << std::endl;
}

void ServerMng::addConnection(Connection* conn) {
    if (conn) {
        std::lock_guard<std::mutex> lock(connectionsMutex);
        connections.push_back(conn);
        std::cout << "Added connection: " << conn->getType() << " on port " << conn->getPort() << std::endl;
    }
}

void ServerMng::removeConnection(Connection* conn) {
    std::lock_guard<std::mutex> lock(connectionsMutex);
    auto it = std::find(connections.begin(), connections.end(), conn);
    if (it != connections.end()) {
        (*it)->stopServer();
        delete* it;
        connections.erase(it);
        std::cout << "Removed connection" << std::endl;
    }
}

void ServerMng::processRequests() {
    std::lock_guard<std::mutex> lock(connectionsMutex);
    for (auto* conn : connections) {
        if (conn->isServerRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void ServerMng::attachController(std::string name, Controller* ctrl) {
    if (ctrl) {
        controllers[name] = ctrl;
        loader->registerObject(nullptr, name); // Register controller
        std::cout << "Attached controller: " << name << std::endl;
    }
}

void ServerMng::removeController(std::string name) {
    auto it = controllers.find(name);
    if (it != controllers.end()) {
        delete it->second;
        controllers.erase(it);
        std::cout << "Removed controller: " << name << std::endl;
    }
}

Connection* ServerMng::getConnection(int id) {
    std::lock_guard<std::mutex> lock(connectionsMutex);
    if (id >= 0 && id < static_cast<int>(connections.size())) {
        return connections[id];
    }
    return nullptr;
}

ChallMng* ServerMng::getChallMng(std::string contestId) {
    for (auto* chall : challMngs) {
        //if (chall->getContestId() == contestId) {
            //return chall;
        //}
    }
    return nullptr;
}

void ServerMng::addToken(const std::string& token)
{
	std::lock_guard<std::mutex> lock(instanceMutex);
	auto it = std::find(tokens.begin(), tokens.end(), token);
	if (it == tokens.end()) {
		tokens.push_back(token);
		std::cout << "Added token: " << token << std::endl;
	}
	else {
		std::cout << "Token already exists: " << token << std::endl;
	}
}

void ServerMng::removeToken(const std::string& token)
{
	std::lock_guard<std::mutex> lock(instanceMutex);
	auto it = std::remove(tokens.begin(), tokens.end(), token);
	if (it != tokens.end()) {
		tokens.erase(it, tokens.end());
		std::cout << "Removed token: " << token << std::endl;
	}

}

void ServerMng::runTCPServer(int port, bool useTLS) {
    std::string serverType = useTLS ? "Secure TCP" : "Standard TCP";
    printMessage("Starting " + serverType + " Server on port " + std::to_string(port));

    auto conn = ConnsFactory::createConnection(ConnectionType::TCP, "", port);
    if (!conn) {
        printMessage("Failed to create TCP server on port " + std::to_string(port));
        return;
    }

    if (useTLS && !conn->enableTLS()) {
        printMessage("Failed to enable TLS for " + serverType);
        return;
    }
    if (conn->isTLSEnabled()) {
        conn->setCertificates("server.crt", "server.key");
    }

    auto handler = [this](const std::string& data, Connection* client) {
        try {
            json j = json::parse(data);
            std::string controllerName = j["action"].get<std::string>().substr(0, j["action"].get<std::string>().find('_'));
            auto it = controllers.find(controllerName);
            if (it != controllers.end()) {
                it->second->handleRequest(data, client);
            }
            else {
                controllers["default"]->handleRequest(data, client);
            }
        }
        catch (const std::exception& e) {
            client->send("ERROR: Invalid request format");
        }
        };
    if (!conn->startListening(handler)) {
        printMessage("Failed to start " + serverType + " server on port " + std::to_string(port));
        return;
    }

    printMessage(serverType + " Server running on port " + std::to_string(port) + ", TLS: " +
        std::string(conn->isTLSEnabled() ? "enabled" : "disabled"));
    addConnection(conn.release());
}

void ServerMng::runUDPServer(int port) {
    printMessage("Starting UDP Server on port " + std::to_string(port));

    auto conn = ConnsFactory::createConnection(ConnectionType::UDP, "", port);
    if (!conn) {
        printMessage("Failed to create UDP server on port " + std::to_string(port));
        return;
    }

    auto handler = [this](const std::string& data, Connection* client) {
        try {
            json j = json::parse(data);
            std::string controllerName = j["action"].get<std::string>().substr(0, j["action"].get<std::string>().find('_'));
            auto it = controllers.find(controllerName);
            if (it != controllers.end()) {
                it->second->handleRequest(data, client);
            }
            else {
                controllers["default"]->handleRequest(data, client);
            }
        }
        catch (const std::exception& e) {
            client->send("ERROR: Invalid request format");
        }
        };

    if (!conn->startListening(handler)) {
        printMessage("Failed to start UDP server on port " + std::to_string(port));
        return;
    }

    addConnection(conn.release());
    printMessage("UDP Server running on port " + std::to_string(port));
}

void ServerMng::runDownloadServer(int port) {
    printMessage("Starting Download Server on port " + std::to_string(port));

    auto conn = ConnsFactory::createConnection(ConnectionType::TRANSFER, "", port, "", "./server_files");
    if (!conn) {
        printMessage("Failed to create Download server on port " + std::to_string(port));
        return;
    }

    auto handler = [this](const std::string& data, Connection* client) {
        try {
            json j = json::parse(data);
            std::string controllerName = j["action"].get<std::string>().substr(0, j["action"].get<std::string>().find('_'));
            auto it = controllers.find(controllerName);
            if (it != controllers.end()) {
                it->second->handleRequest(data, client);
            }
            else {
                controllers["default"]->handleRequest(data, client);
            }
        }
        catch (const std::exception& e) {
            client->send("ERROR: Invalid request format");
        }
        };

    if (!conn->startListening(handler)) {
        printMessage("Failed to start Download server on port " + std::to_string(port));
        return;
    }

    addConnection(conn.release());
    printMessage("Download Server running on port " + std::to_string(port));
}
