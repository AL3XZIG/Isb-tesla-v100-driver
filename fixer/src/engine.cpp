#include "isb/fixer/engine.hpp"
#include <algorithm>

namespace isb::fixer {

void FixEngine::register_rule(FixRule rule) {
    rules_.push_back(std::move(rule));
}

Plan FixEngine::evaluate(const MatchContext& ctx) const {
    Plan plan;
    plan.id = "fix-plan";
    plan.dry_run = true;
    
    // Sort rules by priority (higher first) for deterministic evaluation.
    std::vector<FixRule> sorted_rules = rules_;
    std::sort(sorted_rules.begin(), sorted_rules.end(),
              [](const FixRule& a, const FixRule& b) {
                  return a.priority > b.priority;
              });
    
    for (const auto& rule : sorted_rules) {
        TriState state = evaluate_rule_predicates(rule, ctx);
        
        RuleMatch match;
        match.rule_id = rule.id;
        match.match_state = state;
        match.evidence = "evaluated against context: " + ctx.gpu_model;
        
        if (state == TriState::True) {
            match.actions = rule.actions;
            match.reason = "all predicates matched";
            plan.matched_rules.push_back(std::move(match));
        } else if (state == TriState::False) {
            match.reason = "predicates did not match";
            plan.skipped_rules.push_back(rule.id);
        } else {
            // Unknown
            match.reason = "insufficient evidence to determine match";
            plan.unknown_rules.push_back(rule.id);
        }
    }
    
    return plan;
}

std::vector<ActionResult> Executor::execute(const Plan& plan, bool approved) const {
    std::vector<ActionResult> results;
    
    if (!approved) {
        for (const auto& match : plan.matched_rules) {
            for (const auto& action : match.actions) {
                results.push_back({action.id, false, "execution requires approval", false});
            }
        }
        return results;
    }
    
    if (plan.dry_run) {
        for (const auto& match : plan.matched_rules) {
            for (const auto& action : match.actions) {
                results.push_back({action.id, false, "dry-run mode: no mutation performed", false});
            }
        }
        return results;
    }
    
    // In real execution mode, actions would be applied here.
    // Since we don't have a real backend, we report unimplemented.
    for (const auto& match : plan.matched_rules) {
        for (const auto& action : match.actions) {
            results.push_back({action.id, false, "action execution not implemented for this backend", false});
        }
    }
    
    return results;
}

std::vector<ActionResult> Executor::rollback(const Plan& /*plan*/) const {
    std::vector<ActionResult> results;
    // Rollback requires tracking of actually applied actions.
    // In the current read-only implementation, nothing was applied.
    results.push_back({"rollback", false, "no actions were applied; nothing to roll back", false});
    return results;
}

} // namespace isb::fixer
