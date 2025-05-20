#include "pch.h"
#include "Observable.h"
#include "Observer.h"

Observable::Observable() {}

Observable::~Observable() {}

void Observable::addObserver(Observer* observer) {
    if (observer) {
        std::lock_guard<std::mutex> lock(observerMutex);
        observers.push_back(observer);
    }
}

void Observable::removeObserver(Observer* observer) {
    if (observer) {
        std::lock_guard<std::mutex> lock(observerMutex);
        auto it = std::find(observers.begin(), observers.end(), observer);
        if (it != observers.end()) {
            observers.erase(it);
        }
    }
}

void Observable::notifyObservers(const std::string& message) {
    std::lock_guard<std::mutex> lock(observerMutex);
    for (auto* observer : observers) {
        observer->update(message);
    }
}