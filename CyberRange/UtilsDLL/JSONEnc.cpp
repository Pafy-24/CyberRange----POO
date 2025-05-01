#include "pch.h"
#include "JSONEnc.h"
#include <sstream>
#include <iomanip>

JSONEnc::JSONEnc(bool pretty) : prettyPrint(pretty), escapeUnicode(false)
{
}

std::string JSONEnc::encode(std::map<std::string, JSONValue> data)
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

        oss << (prettyPrint ? "  " : "") << "\"" << escapeString(pair.first) << "\": ";

        // Handle different value types using std::visit
        std::visit([this, &oss](const auto& value) {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, std::string>)
            {
                oss << "\"" << escapeString(value) << "\"";
            }
            else if constexpr (std::is_same_v<T, std::vector<std::string>>)
            {
                oss << encodeArray(value);
            }
            }, pair.second);
    }

    oss << (prettyPrint ? "\n}" : "}");
    return oss.str();
}

std::string JSONEnc::encodeArray(const std::vector<std::string>& array)
{
    std::ostringstream oss;
    oss << (prettyPrint ? "[\n" : "[");

    bool first = true;
    for (const auto& value : array)
    {
        if (!first)
        {
            oss << (prettyPrint ? ",\n" : ",");
        }
        first = false;

        oss << (prettyPrint ? "    " : "") << "\"" << escapeString(value) << "\"";
    }

    oss << (prettyPrint ? "\n  ]" : "]");
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