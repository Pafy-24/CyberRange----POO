#pragma once
#include <string>
#include <map>

class Observer {
public:
    virtual void update(std::map<std::string, std::string> data) = 0;
    virtual std::string getObserverType() = 0;
    virtual ~Observer() = default;
};
