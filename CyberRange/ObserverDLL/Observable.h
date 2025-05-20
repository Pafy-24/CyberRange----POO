#pragma once
#include "DLL.h"
#include <vector>
#include <mutex>
#include <string>

class Observer;

class OBSERVER_API Observable {
private:
    std::vector<Observer*> observers;
    std::mutex observerMutex;
protected:

    void print(const std::string& message) { notifyObservers("\t" + message); }
    void printLog(const std::string& message) { notifyObservers("[LOG]\t" + message); }
    void printInfo(const std::string& message) { notifyObservers("[INFO]\t" + message); }
    void printError(const std::string& message) { notifyObservers("[ERROR]\t" + message); }
    void printWarning(const std::string& message) { notifyObservers("[Warning]\t"+message); }
public:
    Observable();
    virtual ~Observable();

    void addObserver(Observer* observer);

    void removeObserver(Observer* observer);

    void notifyObservers(const std::string& message);
};