#pragma once

#include "isb/fixer/rule.hpp"
#include <string>
#include <vector>

namespace isb::fixer {

/// ExecutionResult for a single action.
struct ActionResult {
    std::string action_id;
    bool success = false;
    std::string message;
    bool rolled_back = false;
};

/// Plan represents the output of rule matching.
struct Plan {
    std::string id;
    bool dry_run = true;
    std::vector<RuleMatch> matched_rules;
    std::vector<std::string> skipped_rules;
    std::vector<std::string> unknown_rules;
};

/// FixEngine evaluates rules against a context and produces a Plan.
class FixEngine {
public:
    void register_rule(FixRule rule);
    Plan evaluate(const MatchContext& ctx) const;
    
private:
    std::vector<FixRule> rules_;
};

/// Executor applies actions from a Plan.
class Executor {
public:
    std::vector<ActionResult> execute(const Plan& plan, bool approved) const;
    std::vector<ActionResult> rollback(const Plan& plan) const;
};

} // namespace isb::fixer
