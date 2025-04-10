#pragma once
#include <string>

class VM {
private:
    std::string name;
    std::string image;
    int memory;
    int cpu;
    bool running;

public:
    VM(std::string name, std::string image);
    bool start();
    bool stop();
    bool restart();
    void setMemory(int mb);
    void setCPU(int cores);
    bool isRunning();
    std::string getStatus();
};
