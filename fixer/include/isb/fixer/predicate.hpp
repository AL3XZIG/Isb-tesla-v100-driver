#pragma once

#include "isb/fixer/tristate.hpp"
#include <string>
#include <functional>

namespace isb::fixer {

/// A predicate evaluates a condition and returns TriState.
/// Unknown means the condition cannot be determined with available evidence.
struct Predicate {
    std::string id;
    std::string description;
    std::function<TriState()> evaluator;
};

/// Evaluate a predicate safely, catching exceptions.
TriState safe_evaluate(const Predicate& p) noexcept;

} // namespace isb::fixer
