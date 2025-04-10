#pragma once
#include "DLL.h"
#include <string>
#include <map>

class UTILS_API JSONEnc {
private:
   bool prettyPrint;
   bool escapeUnicode;

   std::string escapeString(const std::string& input); 
public:
   JSONEnc(bool pretty = false);
   std::string encode(std::map<std::string, std::string> data);
   void setPrettyPrint(bool pretty);
   void setEscapeUnicode(bool escape);
};
