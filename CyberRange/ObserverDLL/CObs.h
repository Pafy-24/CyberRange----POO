#pragma once
#include <string>
#include <map>
#include "Observer.h"
#include "Logger.h"

class CObs : public Observer {
private:
    Logger* logger;
    std::string observerType;
    bool active;

public:
    CObs(Logger& logger);
    void update(std::map<std::string, std::string> data) override;
    void audit();
    void logEvent(std::string event, int level);
    std::string getObserverType() override;
    void setActive(bool active);
    bool isActive();
};
