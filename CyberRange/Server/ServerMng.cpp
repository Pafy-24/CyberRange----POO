#include "pch.h"
#include "ServerMng.h"
#include "CustomSerial.h"
#include "ConnsFactory.h"
#include "DBController.h"
#include "UserController.h"
#include "TeamController.h"
#include "ChallController.h"
#include "ContestController.h"
#include "TabController.h"
#include "Observer.h"
#include "CObs.h"
#include "ObsFactory.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <sstream>

ServerMng* ServerMng::instance = nullptr;

ServerMng* ServerMng::getInstance(int port, const std::string& address) {
    if (instance == nullptr) {
        instance = new ServerMng(port, address);
    }
    return instance;
}

ServerMng::ServerMng(int port, std::string address)
    : Observable(), isRunning(false), port(port), serverAddress(address) {
    dbController = new DBController("sqlserver://administrator:StrongP@ssw0rd!@localhost:1433/CyberRangeDB");
    loader = new Loader();

    srand(time(NULL));
    SecretKey.resize(32);
    std::generate_n(SecretKey.begin(), 32, []() {
        static const char c[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        return c[rand() % (sizeof(c) - 1)]; });

    attachController("UserController", new UserController());
    attachController("TeamController", new TeamController());
    attachController("ChallController", new ChallController());
    attachController("ContestController", new ContestController());
    attachController("TabController", new TabController());
    attachController("default", new CController());

    createDualObserver("Server", "./logs/server");
    createFileObserver("Chall", "./logs/ChallController.log");
	createFileObserver("User", "./logs/UserController.log");
	createFileObserver("Team", "./logs/TeamController.log");
	createFileObserver("Contest", "./logs/ContestController.log");
	createFileObserver("Tab", "./logs/TabController.log");
	createFileObserver("Connection", "./logs/connections");
	createFileObserver("DB", "./logs/DBController.log");
	createFileObserver("Loader", "./logs/Loader.log");
	createFileObserver("ServerMng", "./logs/ServerMng.log");
	getController("UserController")->addObserver(getNamedObserver("User"));
	getController("TeamController")->addObserver(getNamedObserver("Team"));
	getController("ContestController")->addObserver(getNamedObserver("Contest"));
	getController("TabController")->addObserver(getNamedObserver("Tab"));
	getController("ChallController")->addObserver(getNamedObserver("Chall"));
	getController("TeamController")->addObserver(getNamedObserver("Team"));
	getController("ContestController")->addObserver(getNamedObserver("Contest"));
	getController("TabController")->addObserver(getNamedObserver("Tab"));
	getController("default")->addObserver(getNamedObserver("Server"));
	dbController->addObserver(getNamedObserver("DB"));
	loader->addObserver(getNamedObserver("Loader"));
	print("Server manager initialized");


}

ServerMng::~ServerMng() {
    stop();
}

void ServerMng::start() {
    if (isRunning) {
        print("Server manager already running");
        return;
    }
    isRunning = true;

    runTCPServer(port, false);
    runTransferServer(port + 3);

    print("Server manager started");
}

void ServerMng::stop() {
    if (!isRunning) {
        return;
    }
    isRunning = false;

    for (auto* conn : connections) {
        loader->saveUnload(conn);
    }
    connections.clear();
    for (auto* server : servers) {
        server->stopServer();
        delete server;
    }

    for (auto& ctrl : controllers) {
        delete ctrl.second;
    }
    controllers.clear();


    for (auto& observer : namedObservers) {
        removeObserver(observer.second);
        delete observer.second;
    }
    namedObservers.clear();

    delete loader;
    loader = nullptr;

    instance = nullptr;
    print("Server manager stopped");
}

void ServerMng::addConnection(Connection* conn) {
    if (conn) {
        connections.push_back(conn);
        print("Added connection: " + conn->getType() + " on port " + std::to_string(conn->getPort()));

        std::stringstream observerName;
        observerName << "conn_" << conn << "_" << conn->getPort();
        std::stringstream logFileName;
        logFileName << "./logs/connections/conn_" << conn << "_" << conn->getPort() << ".log";

        createFileObserver(observerName.str(), logFileName.str());
        logConnectionMessage(conn, "Connection established: " + conn->getType() + " on port " + std::to_string(conn->getPort()));
    }
}

void ServerMng::removeConnection(Connection* conn) {
    auto it = std::find(connections.begin(), connections.end(), conn);
    if (it != connections.end()) {
        if (loader) {
            loader->saveUnload(conn);
        }

        // Remove the associated FileObserver from namedObservers
        std::stringstream observerName;
        observerName << "conn_" << conn << "_" << conn->getPort();
        logConnectionMessage(conn, "Connection closed: " + conn->getType() + " on port " + std::to_string(conn->getPort()));
        removeNamedObserver(observerName.str());

        (*it)->stopServer();
        delete* it;
        connections.erase(it);
        print("Removed connection");
    }
}

void ServerMng::attachController(std::string name, Controller* ctrl) {
    if (ctrl) {
        controllers[name] = ctrl;
        loader->registerObject(nullptr, name);
        print("Attached controller: " + name);
    }
}

void ServerMng::removeController(std::string name) {
    auto it = controllers.find(name);
    if (it != controllers.end()) {
        delete it->second;
        controllers.erase(it);
        print("Removed controller: " + name);
    }
}

Connection* ServerMng::getConnection(int id) {
    if (id >= 0 && id < static_cast<int>(connections.size())) {
        return connections[id];
    }
    return nullptr;
}

Controller* ServerMng::getController(const std::string& name) {
    auto it = controllers.find(name);
    if (it != controllers.end()) {
        return it->second;
    }
    return nullptr;
}

Contest* ServerMng::getContest(int mngID) {
    auto contests = challMng.getAllContests();
    auto it = contests.find(mngID);
    if (it != contests.end()) {
        return contests[mngID];
    }
    return nullptr;
}

Tab* ServerMng::getTab(int mngID) {
    auto tabs = challMng.getAllTabs();
    auto it = tabs.find(mngID);
    if (it != tabs.end()) {
        return tabs[mngID];
    }
    return nullptr;
}

void ServerMng::addToken(const std::string& token) {
    auto it = std::find(tokens.begin(), tokens.end(), token);
    if (it == tokens.end()) {
        tokens.push_back(token);
        print("Added token: " + token);
    }
    else {
        print("Token already exists: " + token);
    }
}

void ServerMng::removeToken(const std::string& token) {
    auto it = std::remove(tokens.begin(), tokens.end(), token);
    if (it != tokens.end()) {
        tokens.erase(it, tokens.end());
        print("Removed token: " + token);
    }
}

void ServerMng::pushUser(User* user, Connection* conn) {
    if (users[user->GetId()].first == nullptr) {
        users[user->GetId()].first = user;
    }
    users[user->GetId()].second.push_back(conn);
}

void ServerMng::pushTeam(Team* team, Connection* conn) {
    if (teams[team->GetId()].first == nullptr) {
        teams[team->GetId()].first = team;
    }
    teams[team->GetId()].second.push_back(conn);
}

User* ServerMng::findUser(const std::string& usrName_email) {
    for (auto& user : users) {
        if (user.second.first->GetUsername() == usrName_email ||
            user.second.first->GetEmail() == usrName_email) {
            return user.second.first;
        }
    }
    return nullptr;
}

Observer* ServerMng::addNamedObserver(const std::string& name, Observer* observer) {
    if (observer) {
        auto it = namedObservers.find(name);
        if (it != namedObservers.end()) {
            removeObserver(it->second);
            delete it->second;
            namedObservers.erase(it);
        }

        namedObservers[name] = observer;
        addObserver(observer);
        print("Added named observer: " + name);
        return observer;
    }
    return nullptr;
}

Observer* ServerMng::getNamedObserver(const std::string& name) {
    auto it = namedObservers.find(name);
    if (it != namedObservers.end()) {
        return it->second;
    }
    return nullptr;
}

void ServerMng::removeNamedObserver(const std::string& name) {
    auto it = namedObservers.find(name);
    if (it != namedObservers.end()) {
        print("Removing named observer: " + name);
        removeObserver(it->second);
        delete it->second;
        namedObservers.erase(it);
    }
}

Observer* ServerMng::createConsoleObserver(const std::string& name) {
    Observer* observer = ObsFactory::createConsoleObserver();
    if (observer) {
        addNamedObserver(name, observer);
        return observer;
    }
    return nullptr;
}

Observer* ServerMng::createFileObserver(const std::string& name, const std::string& filePath) {
    Observer* observer = ObsFactory::createFileObserver(filePath);
    if (observer) {
        addNamedObserver(name, observer);
        return observer;
    }
    return nullptr;
}

Observer* ServerMng::createDualObserver(const std::string& name, const std::string& filePath) {
    Observer* observer = ObsFactory::createDualObserver(filePath);
    if (observer) {
        addNamedObserver(name, observer);
        return observer;
    }
    return nullptr;
}

void ServerMng::broadcastInfo(const std::string& message) {
    printInfo(message);
}

void ServerMng::broadcastWarning(const std::string& message) {
    printWarning(message);
}

void ServerMng::broadcastError(const std::string& message) {
    printError(message);
}

void ServerMng::logConnectionMessage(Connection* conn, const std::string& message) {
    std::stringstream observerName;
    observerName << "conn_" << conn << "_" << conn->getPort();
    Observer* observer = getNamedObserver(observerName.str());
    if (observer) {
        observer->update(message);
    }
}

void ServerMng::runTCPServer(int port, bool useTLS) {
    std::string serverType = useTLS ? "Secure TCP" : "Standard TCP";
    print("Starting " + serverType + " Server on port " + std::to_string(port));

    auto conn = ConnsFactory::createConnection(ConnectionType::TCP, "", port);
    if (!conn) {
        print("Failed to create TCP server on port " + std::to_string(port));
        return;
    }

    if (useTLS && !conn->enableTLS()) {
        print("Failed to enable TLS for " + serverType);
        return;
    }
    if (conn->isTLSEnabled()) {
        conn->setCertificates("server.crt", "server.key");
    }

    auto handler = [this](const std::string& data, Connection* client) {
        try {
            if (std::find(connections.begin(), connections.end(), client) == connections.end()) {
                addConnection(client);
            }

            logConnectionMessage(client, "Received data: " + data);

            json j = json::parse(data);
            std::string controllerName = j["controller"];
            auto it = controllers.find(controllerName);
            if (it != controllers.end()) {
                it->second->handleRequest(data, client);
                logConnectionMessage(client, "Handled by controller: " + controllerName);
            }
            else {
                controllers["default"]->handleRequest(data, client);
                logConnectionMessage(client, "Handled by default controller");
            }
        }
        catch (const std::exception& e) {
            client->send("ERROR: Invalid request format");
            logConnectionMessage(client, "Error: Invalid request format - " + std::string(e.what()));
        }
        };

    if (!conn->startListening(handler)) {
        print("Failed to start " + serverType + " server on port " + std::to_string(port));
        return;
    }

    print(serverType + " Server running on port " + std::to_string(port) + ", TLS: " +
        std::string(conn->isTLSEnabled() ? "enabled" : "disabled"));
    servers.push_back(conn.release());
}

void ServerMng::runUDPServer(int port) {
    print("Starting UDP Server on port " + std::to_string(port));

    auto conn = ConnsFactory::createConnection(ConnectionType::UDP, "", port);
    if (!conn) {
        print("Failed to create UDP server on port " + std::to_string(port));
        return;
    }

    auto handler = [this](const std::string& data, Connection* client) {
        try {
            if (std::find(connections.begin(), connections.end(), client) == connections.end()) {
                addConnection(client);
            }

            logConnectionMessage(client, "Received data: " + data);

            json j = json::parse(data);
            std::string controllerName = j["action"].get<std::string>().substr(0, j["action"].get<std::string>().find('_'));
            auto it = controllers.find(controllerName);
            if (it != controllers.end()) {
                it->second->handleRequest(data, client);
                logConnectionMessage(client, "Handled by controller: " + controllerName);
            }
            else {
                controllers["default"]->handleRequest(data, client);
                logConnectionMessage(client, "Handled by default controller");
            }
        }
        catch (const std::exception& e) {
            client->send("ERROR: Invalid request format");
            logConnectionMessage(client, "Error: Invalid request format - " + std::string(e.what()));
        }
        };

    if (!conn->startListening(handler)) {
        print("Failed to start UDP server on port " + std::to_string(port));
        return;
    }

    print("UDP Server running on port " + std::to_string(port));
    servers.push_back(conn.release());
}

void ServerMng::runTransferServer(int port) {
    print("Starting Transfer Server on port " + std::to_string(port));

    auto conn = ConnsFactory::createConnection(ConnectionType::TRANSFER, "", port, "", "./server_files");
    if (!conn) {
        print("Failed to create Transfer server on port " + std::to_string(port));
        return;
    }

    if (!conn->startListening(nullptr)) {
        print("Failed to start Transfer server on port " + std::to_string(port));
        return;
    }

    print("Transfer Server running on port " + std::to_string(port));
    servers.push_back(conn.release());
}