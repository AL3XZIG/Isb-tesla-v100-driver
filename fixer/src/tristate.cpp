#include "isb/fixer/tristate.hpp"

namespace isb::fixer {

const char* to_string(TriState state) noexcept {
    switch (state) {
    case TriState::True: return "true";
    case TriState::False: return "false";
    case TriState::Unknown: return "unknown";
    }
    return "unknown";
}

TriState logical_and(TriState lhs, TriState rhs) noexcept {
    // False dominates: False && X = False
    if (lhs == TriState::False || rhs == TriState::False) {
        return TriState::False;
    }
    // True && True = True
    if (lhs == TriState::True && rhs == TriState::True) {
        return TriState::True;
    }
    // Otherwise Unknown propagates
    return TriState::Unknown;
}

TriState logical_or(TriState lhs, TriState rhs) noexcept {
    // True dominates: True || X = True
    if (lhs == TriState::True || rhs == TriState::True) {
        return TriState::True;
    }
    // False || False = False
    if (lhs == TriState::False && rhs == TriState::False) {
        return TriState::False;
    }
    // Otherwise Unknown propagates
    return TriState::Unknown;
}

TriState logical_not(TriState value) noexcept {
    switch (value) {
    case TriState::True: return TriState::False;
    case TriState::False: return TriState::True;
    case TriState::Unknown: return TriState::Unknown;
    }
    return TriState::Unknown;
}

} // namespace isb::fixer
