# krnl

Cross-Platform GPU Computing Library (WebGPU / Dawn)

`krnl` is a lightweight, cross-platform GPU compute library designed on top of Google Dawn (WebGPU) to provide a unified compute abstraction over Vulkan, Metal, and DirectX. It exposes a clean, developer-friendly API for tensors, kernels, and multi-backend execution while providing modular buffer, pipeline, and shader management for efficient GPU compute workloads.

**Status:** development

---

## Key Features

- **Cross-backend compute:** single API that targets Vulkan, Metal, and DirectX via the Dawn WebGPU backend.
- **Modular resource management:** reusable buffer, pipeline, and shader modules for low-friction GPU programming.
- **Tensor & kernel API:** high-level constructs for tensors and kernels to accelerate prototyping and experiments.
- **Multi-backend execution:** switch backends with minimal code changes for portability and performance testing.
- **Developer-focused samples:** `samples/` and `sandbox/` demonstrate common compute patterns and integration.

## Repository Layout

- `krnl/` — library headers and implementation
- `shaders/` — WGSL / shader sources used by the library and samples
- `samples/` — example applications demonstrating usage
- `sandbox/` — experimental applications and quick tests
- `external/` — third-party dependencies (Dawn and vendor projects)

## Prerequisites

- Windows 10/11, macOS, or Linux (platform support depends on Dawn/tooling availability)
- CMake 3.20+
- Compiler/toolchain:
  - Windows: Visual Studio 2022 (C++ workload) or Ninja + appropriate MSVC/clang toolchain
  - macOS: Xcode or Ninja + clang
  - Linux: Ninja + clang/gcc
- Optional: Python for helper scripts in `external/dawn`

## Build (Windows / PowerShell)

Using the Visual Studio generator:

```powershell
mkdir build; cd build
cmake -S .. -B . -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

Using Ninja (cross-platform):

```powershell
cmake -S . -B build -G "Ninja"
cmake --build build --config Release
```

Tip: The repository ships `CMakePresets.json`. Use `cmake --preset <preset-name>` to pick a configured toolchain or build profile.

## Running Samples

After a successful build, binaries appear in the `build` folder (e.g. `build/Release/` on Windows). Try the example apps in `samples/` and `sandbox/`:

```powershell
# from repo root
.
cd build\Release
./samples.exe   # or relevant binary name for your platform
```

## Basic Usage (library overview)

- Create a `krnl::Context` configured for a target backend (Dawn will select the underlying API).
- Allocate buffers via `krnl::Buffer` and manage staging with `krnl::StagingPool` when needed.
- Build compute pipelines / kernels from WGSL shaders and dispatch workloads via `krnl::Pipeline`.
- Use the `Tensor` API for higher-level data structures and kernel bindings.

See `samples/` for concrete usage examples and patterns.

## Development Notes

- Shader sources live in `shaders/` and are usually compiled/loaded at runtime as WGSL.
- Dawn integration and tools live under `external/dawn/` — consult `external/dawn/tools` and README files there for dependency fetching and platform-specific setup.
- The codebase aims to keep backend-specific details behind the Dawn abstraction so that switching between Vulkan/Metal/DirectX remains straightforward.

## Contributing

- Open issues to discuss features or bugs.
- Prefer small, focused pull requests and open an issue first for larger changes.

---
