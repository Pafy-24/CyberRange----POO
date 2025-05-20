#include "pch.h"
#include "ObsFactory.h"
#include <stdexcept>

Observer* ObsFactory::createObserver(OutputType type, const std::string& fileName) {
    switch (type) {
    case OutputType::CONSOLE:
        return createConsoleObserver();
    case OutputType::FILE:
        return createFileObserver(fileName);
    case OutputType::BOTH:
        return createDualObserver(fileName);
    default:
        throw std::invalid_argument("Observer type not supported");
    }
}

Observer* ObsFactory::createConsoleObserver() {
    return new CObs(OutputType::CONSOLE);
}

Observer* ObsFactory::createFileObserver(const std::string& fileName) {
    return new CObs(OutputType::FILE, fileName);
}

Observer* ObsFactory::createDualObserver(const std::string& fileName) {
    return new CObs(OutputType::BOTH, fileName);
}

