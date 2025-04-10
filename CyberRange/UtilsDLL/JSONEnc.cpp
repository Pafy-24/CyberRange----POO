#include "pch.h"
#include "JSONEnc.h"
#include <sstream>
#include <iomanip>

JSONEnc::JSONEnc(bool pretty) : prettyPrint(pretty), escapeUnicode(false)
{
}

std::string JSONEnc::encode(std::map<std::string, std::string> data)
{
    std::ostringstream oss;
    oss << (prettyPrint ? "{\n" : "{");

    bool first = true;
    for (const auto& pair : data)
    {
        if (!first)
        {
            oss << (prettyPrint ? ",\n" : ",");
        }
        first = false;

        oss << (prettyPrint ? "  " : "") << "\"" << escapeString(pair.first) << "\": \"" << escapeString(pair.second) << "\"";
    }

    oss << (prettyPrint ? "\n}" : "}");
    return oss.str();
}

void JSONEnc::setPrettyPrint(bool pretty)
{
    prettyPrint = pretty;
}

void JSONEnc::setEscapeUnicode(bool escape)
{
    escapeUnicode = escape;
}

std::string JSONEnc::escapeString(const std::string& input)
{
    std::ostringstream oss;
    for (char c : input)
    {
        if (c == '\"' || c == '\\')
        {
            oss << '\\' << c;
        }
        else if (escapeUnicode && (c < 0x20 || c > 0x7E))
        {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)(unsigned char)c;
        }
        else
        {
            oss << c;
        }
    }
    return oss.str();
}