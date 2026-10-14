# Portable embedded software foundation

A small, hardware-independent C99 library and host test harness for the first lab
in a distributed embedded system. The same source-level API can be reused by
later 8051, Arduino UNO, and Raspberry Pi labs without putting device access in
the shared core.

## Layout

```text
include/common.h       Fixed-width types, status flags, and shared interfaces
src/common.c           Flag helpers and deterministic state update
tests/test_common.c    13 passing host tests and an isolated expected-failure demo
docs/requirements.md   Requirements and module boundaries
docs/evidence.md       Toolchain versions, test record, and live-demo steps
library.c/.h           Original starter hello() library, preserved as target untitled
```

## Build and run on Windows

Install CMake and a C99-capable C compiler, then run from the repository root:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

To build using the GCC already bundled with CLion on the machine used for this
starter project:

```powershell
$clionBin = 'C:\Program Files\JetBrains\CLion 2026.2.3.1\bin'
$env:Path = (Join-Path $clionBin 'mingw\bin') + ';' +
    (Join-Path $clionBin 'cmake\win\x64\bin') + ';' +
    (Join-Path $clionBin 'ninja\win\x64') + ';' + $env:Path
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=gcc
cmake --build build
ctest --test-dir build --output-on-failure
```

The first CTest case runs the passing suite. The second case executes a
deliberately incorrect expectation; CTest marks it successful only when the
demo program fails as intended. To show that failure directly:

```powershell
.\build\test_common.exe --demo-failure
if ($LASTEXITCODE -ne 1) { throw 'Expected the demo to exit with code 1' }
```

The demo prints an expected/observed state mismatch and exits with code `1`.
This intentional mismatch is isolated from the normal test suite and does not
make the regular build fail.

## Shared types and state behavior

`sample_t` uses a `uint32_t` millisecond timestamp, `uint16_t` status flags,
and an `int16_t` value scaled by ten (`value_x10`). The shared flags occupy
bits 0–7; the upper byte is reserved. Flag helpers reject null pointers, zero
masks, and masks containing reserved bits. `node_status_t` is an example of a
second reusable internal record.

The four states are `INIT`, `NORMAL`, `WARNING`, and `FAULT`. An invalid sample
or state enters `FAULT`; a valid value of at least 100.0 enters `FAULT`, at
least 80.0 enters `WARNING`, and lower values enter `NORMAL`. A fault remains
latched until a valid value is below 60.0. These integer-scaled boundaries
avoid floating-point variation and provide a clear live-demo rule.

## Porting beyond the host

The shared library has no operating-system, Arduino, Raspberry Pi, or
8051-specific includes or calls. To check another supported compiler, configure
a separate build directory with its compiler, build `embedded_core`, and run
the test executable when the target can execute on the host. For MCU targets,
compile the library as an object with the target compiler and record the exact
compiler command, version, warnings, and result in [docs/evidence.md](docs/evidence.md).
Do not transmit C structs directly: define a wire format with explicit field
widths, byte order, and serialization.

Current machine compiler availability and versions are recorded in
[docs/evidence.md](docs/evidence.md); no AVR-GCC or 8051 compiler is assumed.
The starter's `hello()` library and `untitled` static-library target are retained
for compatibility; they are separate from the shared core and test target.

## Example commit sequence

These are suggested commit messages for documenting staged work, not commits
already present in this workspace:

```text
docs: define requirements and portable module boundaries
feat: add fixed-width types and status flag helpers
feat: add deterministic warning and fault state logic
test: cover flag operations, boundaries, and expected failure
docs: record compiler versions and live-demo evidence
```

For a live edit, change `STATE_WARNING_THRESHOLD_X10` in
`include/common.h`, rebuild, and run the tests. The warning-boundary test then
shows exactly which expected result must be updated when the specification
changes. Restore the documented threshold after the demonstration.
