#pragma once

#include "isb/fixer/tristate.hpp"
#include <string>
#include <vector>
#include <map>

namespace isb::fixer {

/// A FixAction represents a single remediation step.
/// Actions are declarative and must be executed by an Executor.
struct Action {
    std::string id;
    std::string description;
    bool reversible = false;
    std::string rollback_description;
    TriState availability = TriState::Unknown;
    std::string reason;
};

/// MatchResult describes how a rule matched (or didn't).
struct RuleMatch {
    std::string rule_id;
    TriState match_state = TriState::Unknown;
    std::vector<Action> actions;
    std::string evidence;
    std::string reason;
};

/// FixRule defines a condition and associated actions.
struct FixRule {
    std::string id;
    std::string description;
    int priority = 0; // Higher priority rules are evaluated first.
    std::vector<Predicate> predicates;
    std::vector<Action> actions;
    std::string documentation;
};

/// MatchContext contains the environment data available for rule evaluation.
struct MatchContext {
    std::string os;
    std::string driver_version;
    std::string gpu_model;
    std::map<std::string, std::string> properties;
};

/// Evaluate all predicates for a rule and return the combined TriState.
TriState evaluate_rule_predicates(const FixRule& rule, const MatchContext& ctx) noexcept;

} // namespace isb::fixer
