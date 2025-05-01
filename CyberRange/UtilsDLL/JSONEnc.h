#pragma once
#include "DLL.h"
#include <string>
#include <map>
#include <vector>
#include <variant>

using JSONValue = std::variant<std::string, std::vector<std::string>>;

class UTILS_API JSONEnc {
private:
	bool prettyPrint;
	bool escapeUnicode;

	std::string escapeString(const std::string& input);
	std::string encodeArray(const std::vector<std::string>& array);

public:
	JSONEnc(bool pretty = false);
	std::string encode(std::map<std::string, JSONValue> data);
	void setPrettyPrint(bool pretty);
	void setEscapeUnicode(bool escape);
};