#include "pch.h"
#include "ServerMng.h"
#include "DBController.h"
#include <iostream>
#include <string>

std::atomic<bool> serverRunning(false);

bool testDB() 
{
    try {
        std::string connStr = "sqlserver://administrator:StrongP@ssw0rd!@localhost:1433/CyberRangeDB";

        DBController* dbCtrl = DBController::getInstance(connStr);
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

void testConns() {
    printMessage("Network Test Server Starting");

    int basePort = 1337;
    ServerMng* serverMgr = ServerMng::getInstance(basePort, "0.0.0.0");
    serverRunning = true;

    serverMgr->start();

    printMessage("All servers started. Press Enter to stop servers.");
    std::cin.get();

    serverRunning = false;
    serverMgr->stop();

    delete serverMgr;
    printMessage("All servers stopped. Test complete.");
}

int main() {
    if (!testDB()) {
        std::cerr << "Database test failed" << std::endl;
        //return 1;
    }
  //  testConns();
    return 0;
}