#pragma once

#include "Observer.h"
#include "CObs.h"
#include <memory>
#include <string>


class OBSERVER_API ObsFactory {
public:
    static Observer* createObserver(OutputType type, const std::string& fileName = "server_log.txt");

    static Observer* createConsoleObserver();
    static Observer* createFileObserver(const std::string& fileName = "server_log.txt");
    static Observer* createDualObserver(const std::string& fileName = "server_log.txt");
};