# Build, test, and demo evidence

## Toolchain record

Versions captured from the current CLion Windows environment on 2026-10-15:

| Tool | Observed version | Use |
|---|---|---|
| GCC | 15.2.0 (MinGW-w64) | Native Windows C99 build |
| CMake | 4.3.1 | Configure and build |
| Ninja | 1.13.2 | Build generator |
| AVR-GCC | Not installed/discovered | No AVR build claimed |
| SDCC (8051) | Not installed/discovered | No 8051 build claimed |
| Clang | Not installed/discovered | No second compiler build claimed |

Check versions again before presenting: `gcc --version`, `cmake --version`, and
`ninja --version`. Record a real AVR-GCC or 8051 compiler version, command,
warnings, and build result here when one is available. The current evidence
meets the host-build requirement; the embedded cross-compile remains an
environment limitation, not a passing test.

## Expected commands and results

```text
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=gcc
cmake --build build
ctest --test-dir build --output-on-failure
```

The host suite contains 13 passing cases covering valid/reserved flags, null
pointers, normal/warning/fault decisions, invalid inputs, and fault recovery.
CTest also invokes `test_common --demo-failure` and expects exit code 1. The
expected-failure CTest case passing means the deliberate failure was observed;
it does not count as an additional normal unit test.

| # | Requirement / condition | Predicted / calculated | Measured / observed | Deviation / error | Engineering interpretation |
|---|---|---|---|---|---|
| 1 | Strict C99 host build | 0 compiler errors/warnings | GCC 15.2.0 completed all four Ninja build steps | None reported | Confirms shared code builds with host toolchain |
| 2 | Host test suite | 13/13 pass | CTest: 2/2 tests passed, including the 13-case suite | None | Covers flags, boundaries, invalid input |
| 3 | Deliberate failure demo | Exit 1; mismatch reported | Expected-failure CTest case passed | Intentional nonzero exit | CTest confirms the mismatch is detected |
| 4 | Second compiler | AVR-GCC or 8051 build | Not available in current environment | Cross-compiler unavailable | Do not claim target portability without compiling |

Retain the full terminal/build log with the submission when required. The
missing target-compiler result is recorded as unavailable, not inferred from
the host build.

## Live modification demo

1. Show the warning boundary (`80.0`) and recovery rule (`below 60.0`) in
   `include/common.h` and `docs/requirements.md`.
2. In `src/common.c`, change the warning comparison from `>=` to `>` while
   leaving the test sample at the documented threshold.
3. Run `cmake --build build` and `ctest --test-dir build --output-on-failure`.
   The warning-boundary test should fail because a value exactly at the
   threshold no longer enters `WARNING`; explain expected-versus-observed state.
4. Keep code and tests consistent with the chosen requirement: either restore
   `>=` for the baseline demo, or formally adopt a strict-boundary rule and
   update the requirement and test together.
5. Run the build and tests again and record the before/after behavior. Never
   leave an unexplained failing test in the submitted baseline.

Rehearsal result: changing `>=` to `>` caused the warning-boundary test to fail
while the other 12 tests passed. The original `>=` rule was restored; rerun the
baseline build and test commands before submission.

Suggested staged commit examples (no commits are created by this project):

```text
docs: define requirements and portable module boundaries
feat: add fixed-width types and status flag helpers
feat: add deterministic warning and fault state logic
test: cover flag operations, boundaries, and expected failure
docs: record compiler versions and live-demo evidence
```
