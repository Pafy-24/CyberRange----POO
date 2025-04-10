#pragma once
#include <string>
#include <map>

class CustomSerial {
private:
    std::string format;
    bool compress;

public:
    std::string serialize(std::map<std::string, std::string> data);
    std::map<std::string, std::string> deserialize(std::string data);
    void setFormat(std::string fmt);
    void enableCompression(bool enable);
    std::string getFormatVersion();
};
