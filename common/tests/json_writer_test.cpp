#include "isb/common/json_writer.hpp"

#include <cassert>
#include <sstream>
#include <string>

using namespace isb::common;

namespace {

std::string quoted(const std::string& s) {
    std::ostringstream out;
    write_json_string(out, s);
    return out.str();
}

} // namespace

int main() {
    // Plain ASCII passes through untouched.
    assert(json_escape("hello world") == "hello world");

    // Structural characters must be escaped.
    assert(json_escape("a\"b") == "a\\\"b");
    assert(json_escape("a\\b") == "a\\\\b");

    // Short escapes for the classic control characters. This is exactly what
    // the duplicated escapers in hub.cpp / throttling.cpp / cli/main.cpp used
    // to get wrong: raw tabs and newlines produced invalid JSON documents.
    assert(json_escape("\b") == "\\b");
    assert(json_escape("\f") == "\\f");
    assert(json_escape("\n") == "\\n");
    assert(json_escape("\r") == "\\r");
    assert(json_escape("\t") == "\\t");

    // Any other control byte becomes \u00xx (lowercase hex, zero padded).
    assert(json_escape(std::string(1, '\x01')) == "\\u0001");
    assert(json_escape(std::string(1, '\x1f')) == "\\u001f");

    // Quoting helpers agree with each other.
    assert(quoted("x\ny") == "\"x\\ny\"");
    assert(quoted_json_string("x\ny") == quoted("x\ny"));

    // String arrays: empty, single, multiple.
    {
        std::ostringstream out;
        write_json_string_array(out, {});
        assert(out.str() == "[]");
    }
    {
        std::ostringstream out;
        write_json_string_array(out, {"a\tb"});
        assert(out.str() == "[\"a\\tb\"]");
    }
    {
        std::ostringstream out;
        write_json_string_array(out, {"a", "b\"c"});
        assert(out.str() == "[\"a\",\"b\\\"c\"]");
    }

    // Optional scalars serialize as value or null.
    {
        std::ostringstream out;
        write_json_optional(out, std::optional<int>{7});
        assert(out.str() == "7");
    }
    {
        std::ostringstream out;
        write_json_optional(out, std::optional<int>{});
        assert(out.str() == "null");
    }

    // Booleans use JSON literals.
    {
        std::ostringstream out;
        write_json_bool(out, true);
        write_json_bool(out, false);
        assert(out.str() == "truefalse");
    }

    // Field helper composes key + escaped value.
    {
        std::ostringstream out;
        json_field(out, "driver", "mock\ndriver-0");
        assert(out.str() == "\"driver\":\"mock\\ndriver-0\"");
    }

    // Generic array with a custom element writer.
    {
        std::ostringstream out;
        const std::vector<std::string> values{"a", "b"};
        write_json_array(out, values, [](const std::string& v) { return quoted_json_string(v); });
        assert(out.str() == "[\"a\",\"b\"]");
    }

    return 0;
}
