# ISB V100 Hub — AI Agent Workflow

This is the standard workflow for Codex, Qwen Coder, Claude Code and other coding agents.

## Before coding

1. Inspect current branch and repository status.
2. Read README.md and the relevant docs.
3. Inspect the target module.
4. Search for existing interfaces and callers.
5. Inspect tests.
6. Inspect CMake targets and dependency propagation.
7. Determine whether the requested behavior is real, mock, experimental or blocked.

Never implement from a task description alone when repository code can answer the question.

## Ownership test

For every change answer:

- Which module owns this behavior?
- Is an interface already present?
- Is this provider observation, CAL capability, Hub orchestration, FixEngine rule, GUI presentation or external integration?
- Which existing callers are affected?
- What must remain independent?

Extend existing boundaries instead of creating duplicates.

## Provider rules

Providers access external APIs/runtimes and translate observations/errors.

Providers must not contain GUI policy, profile policy or hidden compatibility workarounds.

Provider results must preserve availability and provenance.

## Hub rules

Hub coordinates user-facing operations and transactions.

Hub may validate state, create plans, call FixEngine/providers, track rollback and produce structured results.

Hub must not become a dumping ground for raw NVML/Vulkan/DXGI implementation.

## GUI rules

GUI consumes Hub contracts.

GUI must not:

- call NVML/CUDA/Vulkan/DXGI directly;
- duplicate optimization logic;
- edit registry directly;
- patch game files directly;
- report success before verification.

## FixEngine rules

FixEngine remains the single workaround/rule engine.

New compatibility rules belong there unless clearly outside its responsibility.

Rules must be deterministic, explainable, testable and reversible where possible.

## External integration rules

For OptiScaler, DXVK, VKD3D or similar:

1. inspect upstream version;
2. inspect license;
3. record provenance;
4. define integration boundary;
5. implement detection;
6. implement dry-run;
7. implement backup;
8. implement apply;
9. verify;
10. implement rollback;
11. only then consider bundled/offline distribution.

Never silently vendor or copy upstream code.

## Build discipline

For source/CMake changes run a clean build:

rm -rf build
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure

Also run:

git diff --check

Do not commit build directories, CMake caches, generated files or machine-specific absolute paths.

## Public headers

Public headers must be self-contained.

If target A consumes a public header from target B:

- B exports the include directory;
- A links the required target;
- clean builds resolve the include;
- tests exercise the real dependency graph.

Never rely on accidental transitive include paths.

## Unknown/error handling

Test:

- available;
- unavailable;
- unsupported;
- unknown;
- permission denied;
- provider error;
- mock/synthetic.

Do not turn errors into zero or success.

## Hardware safety

Any mutation must use:

READ -> VALIDATE -> PLAN -> USER APPROVAL -> APPLY -> READ BACK -> VERIFY

Use backup and rollback where possible.

No blind overclocking, blind power-limit changes, silent registry edits or silent anti-cheat modifications.

## PR discipline

A good agent change is:

- focused;
- tested;
- documented;
- explicit about real vs mock behavior;
- explicit about remaining limitations.

Final report should list:

- changed files;
- reused interfaces;
- added interfaces/adapters;
- real implementation;
- mock implementation;
- tests and commands;
- warnings;
- untested hardware/OS cases;
- remaining dependencies.

## Blocked is a valid result

If an implementation is blocked, record:

- blocker;
- evidence;
- missing dependency;
- smallest required interface;
- next experiment or hardware test.

Do not fabricate a PASS.

## Documentation synchronization

After meaningful changes update:

- docs/CURRENT_IMPLEMENTATION.md;
- docs/TASKS.md when roadmap status changes;
- feature-specific documentation;
- README when user-facing direction changes.

Documentation is part of implementation.
