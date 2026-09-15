#pragma once

#include "isb/cal/capabilities.hpp"

#include <string>
#include <vector>

namespace isb::cal {

enum class ValidationSeverity {
    Error,
};

struct ValidationIssue {
    std::string code;
    ValidationSeverity severity = ValidationSeverity::Error;
    std::string message;
};

struct ValidationResult {
    std::vector<ValidationIssue> issues;

    [[nodiscard]] bool valid() const noexcept { return issues.empty(); }
};

/// Validate provider-neutral cross-field invariants.
///
/// Validation never invents unknown values. Fields that are Unknown or absent
/// are accepted unless another populated field makes the combination
/// contradictory. V100-specific invariants are checked when the GPU variant
/// explicitly identifies V100 SXM2 or V100 PCIe.
[[nodiscard]] ValidationResult validate(const GpuCapabilities& capabilities);

} // namespace isb::cal
