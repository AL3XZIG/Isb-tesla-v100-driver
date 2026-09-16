#ifndef ISB_VERIFICATION_ASSERTIONS_HPP
#define ISB_VERIFICATION_ASSERTIONS_HPP

#include "isb/verification/types.hpp"

namespace isb::verification {

AssertionResult evaluateAssertion(
    const Assertion& assertion,
    const BackendExecutionResult& execution,
    const Snapshot& snapshot);

} // namespace isb::verification

#endif // ISB_VERIFICATION_ASSERTIONS_HPP
