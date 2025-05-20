#include "pch.h"
#include "ServerMng.h"
#include "DBController.h"
#include "Orchestrator.h"
#include "Docker.h"
#include "Observer.h"
#include "ObsFactory.h"
#include <iostream>
#include <string>

std::atomic<bool> serverRunning(false);

bool testDB() {
    try {
        std::string connStr = "sqlserver://administrator:StrongP@ssw0rd!@localhost:1433/CyberRangeDB";

        DBController* dbCtrl = new DBController(connStr);
        if (!dbCtrl->connect()) {
            std::cerr << "Connection failed" << std::endl;
            return false;
        }

        std::string query = "SELECT * FROM Users;";
        auto results = dbCtrl->executeQuery(query);

        std::cout << "Results:\n";
        for (const auto& row : results) {
            for (const auto& col : row) {
                std::cout << col.first << ": " << col.second << " ";
            }
            std::cout << std::endl;
        }

        dbCtrl->disconnect();
        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return false;
    }
}

void testDocker() {
    Orchestrator* docker = new Docker(1, 2, "dockerfile", 10);
    if (!docker->deploy()) {
        std::cerr << "Failed to deploy Docker container" << std::endl;
        return;
    }
    if (!docker->start()) {
        std::cerr << "Failed to start Docker container" << std::endl;
        return;
    }
    std::cin.get();
    std::cout << "Docker container started successfully" << std::endl;
    if (!docker->stop()) {
        std::cerr << "Failed to stop Docker container" << std::endl;
        return;
    }
    std::cout << "Docker container stopped successfully" << std::endl;
    delete docker;
}

void testConns() {
    printMessage("Network Test Server Starting");

    int basePort = 1337;
    ServerMng* serverMgr = ServerMng::getInstance(basePort, "0.0.0.0");

    Observer* consoleObserver = ObsFactory::createConsoleObserver();
    serverMgr->addObserver(consoleObserver);

    Observer* fileObserver = ObsFactory::createFileObserver("./logs/server_activities.log");
    serverMgr->addObserver(fileObserver);

    Observer* dualObserver = ObsFactory::createDualObserver("./logs/complete_server.log");
    serverMgr->addObserver(dualObserver);

    serverRunning = true;
    serverMgr->start();

    printMessage("All servers started. Press Enter to stop servers.");
    std::cin.get();

    serverRunning = false;
    serverMgr->stop();

    delete consoleObserver;
    delete fileObserver;
    delete dualObserver;

    printMessage("All servers stopped. Test complete.");
}

int main() {
    // testDocker();
     //if (!testDB()) {
     //    std::cerr << "Database test failed" << std::endl;
     //    //return 1;
     //}
    testConns();
    return 0;
}