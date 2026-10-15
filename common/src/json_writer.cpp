#include "isb/common/json_writer.hpp"

#include <iomanip>
#include <sstream>

namespace isb {
namespace common {

std::string json_escape(const std::string& value) {
    std::string result;
    result.reserve(value.size() + 2);
    for (const unsigned char c : value) {
        switch (c) {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:
            if (c < 0x20U) {
                std::ostringstream hex;
                hex << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                    << static_cast<unsigned int>(c);
                result += hex.str();
            } else {
                result += static_cast<char>(c);
            }
        }
    }
    return result;
}

std::string quoted_json_string(const std::string& value) {
    std::string result;
    result.reserve(value.size() + 2);
    result += '"';
    result += json_escape(value);
    result += '"';
    return result;
}

void write_json_string(std::ostringstream& out, const std::string& value) {
    out << '"' << json_escape(value) << '"';
}

void write_json_bool(std::ostringstream& out, bool value) {
    out << (value ? "true" : "false");
}

void write_json_string_array(std::ostringstream& out, const std::vector<std::string>& values) {
    out << '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) out << ',';
        write_json_string(out, values[i]);
    }
    out << ']';
}

} // namespace common
} // namespace isb
