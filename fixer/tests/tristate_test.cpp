#include "isb/fixer/tristate.hpp"
#include <cassert>
#include <string>

int main() {
    using namespace isb::fixer;
    
    // Test to_string
    assert(std::string(to_string(TriState::True)) == "true");
    assert(std::string(to_string(TriState::False)) == "false");
    assert(std::string(to_string(TriState::Unknown)) == "unknown");
    
    // Test AND
    assert(logical_and(TriState::True, TriState::True) == TriState::True);
    assert(logical_and(TriState::True, TriState::False) == TriState::False);
    assert(logical_and(TriState::False, TriState::True) == TriState::False);
    assert(logical_and(TriState::False, TriState::False) == TriState::False);
    assert(logical_and(TriState::True, TriState::Unknown) == TriState::Unknown);
    assert(logical_and(TriState::Unknown, TriState::True) == TriState::Unknown);
    assert(logical_and(TriState::False, TriState::Unknown) == TriState::False);
    assert(logical_and(TriState::Unknown, TriState::False) == TriState::False);
    assert(logical_and(TriState::Unknown, TriState::Unknown) == TriState::Unknown);
    
    // Test OR
    assert(logical_or(TriState::True, TriState::True) == TriState::True);
    assert(logical_or(TriState::True, TriState::False) == TriState::True);
    assert(logical_or(TriState::False, TriState::True) == TriState::True);
    assert(logical_or(TriState::False, TriState::False) == TriState::False);
    assert(logical_or(TriState::True, TriState::Unknown) == TriState::True);
    assert(logical_or(TriState::Unknown, TriState::True) == TriState::True);
    assert(logical_or(TriState::False, TriState::Unknown) == TriState::Unknown);
    assert(logical_or(TriState::Unknown, TriState::False) == TriState::Unknown);
    assert(logical_or(TriState::Unknown, TriState::Unknown) == TriState::Unknown);
    
    // Test NOT
    assert(logical_not(TriState::True) == TriState::False);
    assert(logical_not(TriState::False) == TriState::True);
    assert(logical_not(TriState::Unknown) == TriState::Unknown);
    
    return 0;
}
