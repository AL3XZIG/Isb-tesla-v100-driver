#include "isb/providers/nvml/identity.hpp"

#include <cassert>

int main() {
    using isb::providers::nvml::is_tesla_v100_model;
    assert(is_tesla_v100_model("Tesla V100-SXM2-16GB"));
    assert(is_tesla_v100_model("NVIDIA Tesla V100-PCIE-32GB"));
    assert(!is_tesla_v100_model("NVIDIA GeForce RTX 4090"));
    assert(!is_tesla_v100_model("NVIDIA A100-SXM4-40GB"));
    assert(!is_tesla_v100_model("Quadro GV100"));
    assert(!is_tesla_v100_model(""));
    return 0;
}
