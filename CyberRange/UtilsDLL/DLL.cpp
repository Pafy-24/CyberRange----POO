#include "pch.h"
#include "DLL.h"

std::mutex consoleMutex;
std::atomic<bool> serverRunning(false);

void printMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << "[" << std::this_thread::get_id() << "] " << message << std::endl;
}
