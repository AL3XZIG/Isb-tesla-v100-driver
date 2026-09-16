#include "isb/verification/json.hpp"

#include <cstddef>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

namespace isb::verification {
namespace {

std::string escapeJsonString(const std::string& input) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');

    for (const char c : input) {
        const auto byte = static_cast<unsigned char>(c);
        switch (c) {
            case '"': ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default:
                if (byte < 0x20U) {
                    ss << "\\u" << std::setw(4) << static_cast<unsigned int>(byte);
                } else {
                    ss << c;
                }
                break;
        }
    }
    return ss.str();
}

template <typename V, typename ConvFunc>
std::string serializeMapDeterministically(
    const std::map<std::string, V>& src_map,
    ConvFunc value_converter,
    std::size_t indent_level) {
    const std::string entry_indent(indent_level * 2, ' ');
    const std::string closing_indent((indent_level - 1) * 2, ' ');
    std::ostringstream ss;
    ss << "{\n";

    std::size_t count = 0;
    for (const auto& [key, value] : src_map) {
        ss << entry_indent << "\"" << escapeJsonString(key) << "\": "
           << value_converter(value);
        if (++count < src_map.size()) {
            ss << ',';
        }
        ss << '\n';
    }

    ss << closing_indent << '}';
    return ss.str();
}

std::string quoteString(const std::string& value) {
    return "\"" + escapeJsonString(value) + "\"";
}

std::string quoteCapability(CapabilityState state) {
    return quoteString(capabilityStateToString(state));
}

std::string serializeSnapshot(const Snapshot& snapshot, std::size_t indent_level) {
    const std::string indent(indent_level * 2, ' ');
    std::ostringstream ss;
    ss << "{\n";
    ss << indent << "\"api_data\": "
       << serializeMapDeterministically(
            snapshot.api_data,
            [](const std::string& value) { return quoteString(value); },
            indent_level + 1) << ",\n";
    ss << indent << "\"capability_states\": "
       << serializeMapDeterministically(
            snapshot.capability_states,
            [](CapabilityState state) { return quoteCapability(state); },
            indent_level + 1) << ",\n";
    ss << indent << "\"environment_data\": "
       << serializeMapDeterministically(
            snapshot.environment_data,
            [](const std::string& value) { return quoteString(value); },
            indent_level + 1) << ",\n";
    ss << indent << "\"gpu_data\": "
       << serializeMapDeterministically(
            snapshot.gpu_data,
            [](const std::string& value) { return quoteString(value); },
            indent_level + 1) << ",\n";
    ss << indent << "\"metadata\": "
       << serializeMapDeterministically(
            snapshot.metadata,
            [](const std::string& value) { return quoteString(value); },
            indent_level + 1) << '\n';
    ss << std::string((indent_level - 1) * 2, ' ') << '}';
    return ss.str();
}

} // namespace

std::string serializeSnapshotToDeterministicJson(const Snapshot& snapshot) {
    return serializeSnapshot(snapshot, 1);
}

std::string serializeSnapshotDiffToDeterministicJson(const SnapshotDiff& diff) {
    std::ostringstream ss;
    ss << "{\n  \"differences\": {\n";

    const std::map<std::string, const std::map<std::string, SnapshotDiff::FieldDiff>*> categories = {
        {"api_data", &diff.api_data_diffs},
        {"capability_states", &diff.capability_diffs},
        {"environment_data", &diff.environment_diffs},
        {"gpu_data", &diff.gpu_diffs},
        {"metadata", &diff.metadata_diffs}
    };

    std::size_t count = 0;
    for (const auto& [category, values] : categories) {
        ss << "    \"" << category << "\": "
           << serializeMapDeterministically(
                *values,
                [](const SnapshotDiff::FieldDiff& field) {
                    std::ostringstream entry;
                    entry << "{\n"
                          << "          \"actual\": \"" << escapeJsonString(field.actual) << "\",\n"
                          << "          \"baseline\": \"" << escapeJsonString(field.baseline) << "\"\n"
                          << "        }";
                    return entry.str();
                },
                3);
        if (++count < categories.size()) {
            ss << ',';
        }
        ss << '\n';
    }

    ss << "  },\n";
    ss << "  \"has_differences\": " << (diff.has_differences ? "true" : "false") << '\n';
    ss << '}';
    return ss.str();
}

std::string serializeTestResultToDeterministicJson(const TestResult& result) {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"actual_snapshot\": " << serializeSnapshot(result.actual_snapshot, 1) << ",\n";
    ss << "  \"failure_reason\": \"" << escapeJsonString(result.failure_reason) << "\",\n";
    ss << "  \"status\": \"" << escapeJsonString(testStatusToString(result.status)) << "\",\n";
    ss << "  \"test_name\": \"" << escapeJsonString(result.test_name) << "\"\n";
    ss << '}';
    return ss.str();
}

std::string serializeTestSuiteResultToDeterministicJson(const TestSuiteResult& suite_result) {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"failed_count\": " << suite_result.failed_count << ",\n";
    ss << "  \"no_change_count\": " << suite_result.no_change_count << ",\n";
    ss << "  \"passed_count\": " << suite_result.passed_count << ",\n";
    ss << "  \"regression_count\": " << suite_result.regression_count << ",\n";
    ss << "  \"results\": [\n";
    for (std::size_t i = 0; i < suite_result.results.size(); ++i) {
        ss << "    " << serializeTestResultToDeterministicJson(suite_result.results[i]);
        if (i + 1 < suite_result.results.size()) {
            ss << ',';
        }
        ss << '\n';
    }
    ss << "  ],\n";
    ss << "  \"suite_name\": \"" << escapeJsonString(suite_result.suite_name) << "\",\n";
    ss << "  \"timeout_count\": " << suite_result.timeout_count << ",\n";
    ss << "  \"unknown_count\": " << suite_result.unknown_count << '\n';
    ss << '}';
    return ss.str();
}

} // namespace isb::verification
