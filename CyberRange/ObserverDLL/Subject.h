#pragma once
#include <string>
#include <vector>
#include <map>
#include "Observer.h"

template <typename T>
class Subject {
private:
    std::vector<Observer*> observers;
    std::string subjectName;

public:
    Subject(std::string name) : subjectName(name) {}
    
    void attach(Observer* observer) {
        observers.push_back(observer);
    }
    
    void detach(Observer* observer) {
        for (auto it = observers.begin(); it != observers.end(); ++it) {
            if (*it == observer) {
                observers.erase(it);
                break;
            }
        }
    }
    
    void notifyObservers(std::map<std::string, std::string> data) {
        for (auto observer : observers) {
            observer->update(data);
        }
    }
    
    std::vector<Observer*> getObservers() {
        return observers;
    }
    
    std::string getSubjectName() {
        return subjectName;
    }
};
