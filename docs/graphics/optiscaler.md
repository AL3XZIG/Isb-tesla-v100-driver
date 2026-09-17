# OptiScaler compatibility research

## Scope

[OptiScaler](https://github.com/optiscaler/OptiScaler) is an application-side compatibility project that can route supported game upscaler integrations to alternative temporal upscalers. ISB does not embed, reproduce, reverse engineer, or configure OptiScaler. The experimental graphics module only records capability observations that can guide future, separately approved compatibility research.

## Relevant APIs and pipelines

A compatibility investigation may need to observe the application API boundary (typically DirectX 11/12 or Vulkan) and the upscaler backend selected by the application. Candidate compute paths for research include CUDA, Vulkan compute, DirectX compute, FSR, XeSS, and neural-upscaling experiments. API presence is not evidence that an upscaler pipeline is functional; resource formats, motion vectors, depth, exposure, synchronization, and per-application integration must be validated independently.

## V100 capability boundaries

Tesla V100 has Volta Tensor Cores when CAL/NVML records them, and its SM capability should be supplied by CAL or a CUDA runtime probe. It has no RTX RT Cores and no NVIDIA Optical Flow Accelerator. Those facts must be observed from a provider/CAL source, not inferred from this document or a model-name check.

Consequently, CUDA and supported Vulkan/DirectX compute experiments can be considered only after their respective probes report availability. Tensor-oriented research may be considered when Tensor Core availability is observed. FSR/XeSS-style experiments are not promised by this capability layer and require their own API, licence, and functional compatibility validation.

## Not possible without RTX-class hardware support

This repository must report DLSS Frame Generation as unsupported when no Optical Flow Accelerator is observed. RTX ray-tracing workloads require observed RT Core support. Neither condition can be satisfied by a shim, a fake API response, or an ISB driver change. This module deliberately performs no benchmark, workload, driver creation, kernel-module work, GUI work, DLSS implementation, or OptiScaler code integration.
