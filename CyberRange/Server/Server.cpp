#include "pch.h"
#include "ServerMng.h"
#include <iostream>
#include <string>

std::atomic<bool> serverRunning(false);

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
    testConns();
    return 0;
}