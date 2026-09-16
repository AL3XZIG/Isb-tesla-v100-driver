#ifndef ISB_VERIFICATION_JSON_HPP
#define ISB_VERIFICATION_JSON_HPP

#include "isb/verification/snapshot.hpp"
#include "isb/verification/types.hpp"

#include <string>

namespace isb::verification {

std::string serializeSnapshotToDeterministicJson(const Snapshot& snapshot);
std::string serializeTestResultToDeterministicJson(const TestResult& result);
std::string serializeTestSuiteResultToDeterministicJson(const TestSuiteResult& suite_result);
std::string serializeSnapshotDiffToDeterministicJson(const SnapshotDiff& diff);

} // namespace isb::verification

#endif // ISB_VERIFICATION_JSON_HPP
