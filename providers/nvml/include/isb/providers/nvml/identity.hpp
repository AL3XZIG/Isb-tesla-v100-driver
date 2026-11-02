#pragma once

#include <string>

namespace isb::providers::nvml {

// NVML's model string is the evidence available at this layer. Other NVIDIA
// products, including non-Volta models, must not inherit V100/Volta/CC 7.0.
inline bool is_tesla_v100_model(const std::string& model_name) {
    return model_name.find("Tesla V100") != std::string::npos;
}

} // namespace isb::providers::nvml
