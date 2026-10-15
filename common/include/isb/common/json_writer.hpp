#pragma once

// Shared, RFC 8259-safe JSON serialization helpers for the whole project.
//
// Before this header existed, every component carried its own ad-hoc escaper
// (cal/src/capabilities.cpp, hub/src/hub.cpp, performance/src/throttling.cpp,
// cli/main.cpp) and several of them silently produced invalid JSON whenever a
// string contained a tab, newline or other control character (for example a
// process name or an NVML error message). This is the single canonical
// implementation; new code must use it instead of writing local copies.

#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace isb {
namespace common {

// Escapes a string *body* (without surrounding quotes) per RFC 8259:
// \", \\, \b, \f, \n, \r, \t and every other control byte as \u00xx.
std::string json_escape(const std::string& value);

// Returns the escaped string wrapped in quotes: "\"value\"".
std::string quoted_json_string(const std::string& value);

// Writes a quoted, escaped string to the stream: "value".
void write_json_string(std::ostringstream& out, const std::string& value);

// Writes a bool as true/false.
void write_json_bool(std::ostringstream& out, bool value);

// Writes an optional scalar as its JSON value, or null when empty.
template <typename T>
void write_json_optional(std::ostringstream& out, const std::optional<T>& value) {
    if (value) {
        out << *value;
    } else {
        out << "null";
    }
}

// Writes an array of strings: ["a","b"].
void write_json_string_array(std::ostringstream& out, const std::vector<std::string>& values);

// Writes an array using a caller-provided element writer (e.g. a pointer to
// the overloaded json() functions): [elem(elem0), elem(elem1), ...].
template <typename T, typename F>
void write_json_array(std::ostringstream& out, const std::vector<T>& values, const F& element_writer) {
    out << '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) out << ',';
        out << element_writer(values[i]);
    }
    out << ']';
}

inline void json_field(std::ostringstream& out, const char* key, const std::string& value) {
    out << '"' << key << "\":";
    write_json_string(out, value);
}

inline void json_field(std::ostringstream& out, const char* key, bool value) {
    out << '"' << key << "\":";
    write_json_bool(out, value);
}

template <typename T>
void json_field(std::ostringstream& out, const char* key, const std::optional<T>& value) {
    out << '"' << key << "\":";
    write_json_optional(out, value);
}

inline void json_field(std::ostringstream& out, const char* key, const std::vector<std::string>& value) {
    out << '"' << key << "\":";
    write_json_string_array(out, value);
}

} // namespace common
} // namespace isb

// Convenience aliases so existing call sites inside isb::hub / isb::performance
// can pick up the shared helpers without long qualification.
namespace isb {
using common::json_escape;
using common::quoted_json_string;
using common::write_json_string;
using common::write_json_string_array;
} // namespace isb
