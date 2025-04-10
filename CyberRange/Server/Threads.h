#pragma once
#include <string>

class Threads {
public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void run() = 0;
    virtual bool isActive() = 0;
    virtual ~Threads() = default;
};
