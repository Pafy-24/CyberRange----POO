#pragma once
#include "DLL.h"
#include <string>
#include <fstream>

enum class OutputType {
    CONSOLE,
    FILE,
    BOTH
};


class OBSERVER_API Observer {
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
};

