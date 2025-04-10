#pragma once
#include <string>
#include <map>

class JSONEnc {
private:
    bool prettyPrint;
    bool escapeUnicode;

public:
    JSONEnc(bool pretty = false);
    std::string encode(std::map<std::string, std::string> data);
    void setPrettyPrint(bool pretty);
    void setEscapeUnicode(bool escape);
};
