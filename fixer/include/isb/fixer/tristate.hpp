#pragma once

#include <string>

namespace isb::fixer {

/// Three-valued logic for rule evaluation.
/// Unknown must never be treated as a positive match.
enum class TriState {
    True,
    False,
    Unknown,
};

/// Convert TriState to string for logging and provenance.
const char* to_string(TriState state) noexcept;

/// Logical AND for TriState values.
/// Unknown propagates: True && Unknown = Unknown, False && Unknown = False.
TriState logical_and(TriState lhs, TriState rhs) noexcept;

/// Logical OR for TriState values.
/// Unknown propagates: False || Unknown = Unknown, True || Unknown = True.
TriState logical_or(TriState lhs, TriState rhs) noexcept;

/// Negation for TriState values.
TriState logical_not(TriState value) noexcept;

} // namespace isb::fixer
