#include "isb/fixer/rule.hpp"

namespace isb::fixer {

TriState evaluate_rule_predicates(const FixRule& rule, const MatchContext& /*ctx*/) noexcept {
    if (rule.predicates.empty()) {
        return TriState::Unknown;
    }
    
    // All predicates must be True for the rule to match.
    // False or Unknown in any predicate prevents a positive match.
    TriState combined = TriState::True;
    for (const auto& pred : rule.predicates) {
        TriState result = safe_evaluate(pred);
        combined = logical_and(combined, result);
        
        // Early exit on False: once False, cannot become True.
        if (combined == TriState::False) {
            return TriState::False;
        }
    }
    return combined;
}

} // namespace isb::fixer
