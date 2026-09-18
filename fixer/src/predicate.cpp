#include "isb/fixer/predicate.hpp"

namespace isb::fixer {

TriState safe_evaluate(const Predicate& p) noexcept {
    try {
        if (p.evaluator) {
            return p.evaluator();
        }
        return TriState::Unknown;
    } catch (...) {
        // Exceptions are treated as Unknown, not False.
        // This prevents unexpected errors from being misinterpreted as negative matches.
        return TriState::Unknown;
    }
}

} // namespace isb::fixer
