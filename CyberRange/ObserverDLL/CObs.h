#pragma once
#include "Observer.h"

class CObs : public Observer {
private:
    OutputType outputType;
    std::string logFileName;
    std::ofstream logFile;

public:

    CObs(OutputType type = OutputType::CONSOLE, const std::string& fileName = "server_log.txt");
    virtual ~CObs();


    void update(const std::string& message) override;


    void setOutputType(OutputType type);


    void setLogFileName(const std::string& fileName);
};