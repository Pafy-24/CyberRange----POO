#pragma once
#include <string>
#include "Threads.h"

class CThreads : public Threads {
private:
    int id;
    bool isRunning;
    std::string threadName;
    int priority;
protected:
    void* threadFunction;

public:
    CThreads(int id, std::string name);
    void start() override;
    void stop() override;
    void run() override;
    bool isActive() override;
    int getId();
    std::string getName();
    void setPriority(int prio);
};
