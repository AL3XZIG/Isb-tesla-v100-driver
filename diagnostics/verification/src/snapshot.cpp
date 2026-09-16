#include "isb/verification/snapshot.hpp"

namespace isb::verification {
namespace {

void compareStringMap(
    const std::map<std::string, std::string>& baseline,
    const std::map<std::string, std::string>& actual,
    std::map<std::string, SnapshotDiff::FieldDiff>& out) {
    auto b = baseline.begin();
    auto a = actual.begin();

    while (b != baseline.end() || a != actual.end()) {
        if (a == actual.end() || (b != baseline.end() && b->first < a->first)) {
            out[b->first] = {b->second, "<missing>"};
            ++b;
        } else if (b == baseline.end() || a->first < b->first) {
            out[a->first] = {"<missing>", a->second};
            ++a;
        } else {
            if (b->second != a->second) {
                out[b->first] = {b->second, a->second};
            }
            ++b;
            ++a;
        }
    }
}

void compareCapabilityMap(
    const std::map<std::string, CapabilityState>& baseline,
    const std::map<std::string, CapabilityState>& actual,
    std::map<std::string, SnapshotDiff::FieldDiff>& out) {
    auto b = baseline.begin();
    auto a = actual.begin();

    while (b != baseline.end() || a != actual.end()) {
        if (a == actual.end() || (b != baseline.end() && b->first < a->first)) {
            out[b->first] = {capabilityStateToString(b->second), "<missing>"};
            ++b;
        } else if (b == baseline.end() || a->first < b->first) {
            out[a->first] = {"<missing>", capabilityStateToString(a->second)};
            ++a;
        } else {
            if (b->second != a->second) {
                out[b->first] = {
                    capabilityStateToString(b->second),
                    capabilityStateToString(a->second)};
            }
            ++b;
            ++a;
        }
    }
}

} // namespace

SnapshotDiff compareSnapshots(const Snapshot& baseline, const Snapshot& actual) {
    SnapshotDiff diff;

    compareStringMap(baseline.environment_data, actual.environment_data, diff.environment_diffs);
    compareStringMap(baseline.gpu_data, actual.gpu_data, diff.gpu_diffs);
    compareStringMap(baseline.api_data, actual.api_data, diff.api_data_diffs);
    compareCapabilityMap(baseline.capability_states, actual.capability_states, diff.capability_diffs);
    compareStringMap(baseline.metadata, actual.metadata, diff.metadata_diffs);

    diff.has_differences = !diff.environment_diffs.empty()
        || !diff.gpu_diffs.empty()
        || !diff.api_data_diffs.empty()
        || !diff.capability_diffs.empty()
        || !diff.metadata_diffs.empty();

    return diff;
}

} // namespace isb::verification
