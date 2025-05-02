#include "pch.h"
#include "ServerMng.h"
#include "DBConn.h"
#include <iostream>
#include <string>

std::atomic<bool> serverRunning(false);
bool testDB() {
    try {
        std::string connStr = "sqlserver://administrator:StrongP@ssw0rd!@localhost:1433/CyberRangeDB";

        DBConn db(connStr);

        if (!db.connect()) {
            std::cerr << "Connection failed" << std::endl;
            return 1;
        }


        std::string query = "SELECT * FROM Users;";
        auto results = db.fetchAll(query);

        std::cout << "Results:\n";
        for (const auto& row : results) {
            for (const auto& col : row) {
                std::cout << col.first << ": " << col.second << " ";
            }
            std::cout << std::endl;
        }

        db.disconnect();
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return -1;
    }
    return 1;
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
    testDB();
    testConns();
  //  int basePort = 1337;
 //   ServerMng* serverMgr = ServerMng::getInstance(basePort, "0.0.0.0");
 //   serverRunning = true;
  //  serverMgr->start();

    return 0;
}