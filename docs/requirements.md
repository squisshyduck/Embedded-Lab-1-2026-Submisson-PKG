# Requirements and module boundaries

## Measurable requirements

1. Compile the shared implementation as strict C99 with warnings enabled.
2. Use fixed-width integer types for timestamps, flags, sample values, and node
   identifiers; document each field's units and scaling.
3. Define eight named status flags in bits 0–7 and reject attempts to modify
   reserved bits.
4. Provide set, clear, toggle, and test operations that work on individual or
   combined valid flags.
5. Model initialization, normal operation, warning, and fault as explicit
   system states.
6. Make state decisions deterministic at the 60.0, 80.0, and 100.0 boundaries;
   invalid input must fail safe to `FAULT`.
7. Pass at least eight host tests and verify one intentionally incorrect
   expectation fails without breaking the normal test run.
8. Keep device registers, GPIO, timers, and network I/O outside the shared
   core.

## Module/dependency diagram

```text
tests/test_common.c
        |
        v
include/common.h <--- src/common.c
        |
        +--- no hardware, OS, or protocol dependencies

future adapter: 8051 drivers ---+
future adapter: Arduino drivers +-- include common API; serialize explicitly
future adapter: Pi gateway -----+
```

## Portability assumptions and risks

| Type | Shared definition | Windows GCC (observed) | Arduino UNO / AVR | 8051 toolchain |
|---|---|---|---|---|
| `char` | Implementation-defined signedness | Compiler/target-defined | Compiler/target-defined | Compiler/target-defined |
| `int` | At least 16 bits in C | Commonly 32 bits | Commonly 16 bits | Often 16 bits |
| `long` | At least 32 bits in C | Commonly 32 bits on Windows | Commonly 32 bits | Compiler-dependent |
| Pointer | Address-space representation | Host-width pointer | Commonly 16-bit data pointer | Memory-model dependent |
| `uint16_t` | Exactly 16 bits when provided | 16 bits | 16 bits | 16 bits when provided |
| `uint32_t` | Exactly 32 bits when provided | 32 bits | 32 bits | May be slower or unsupported by a limited compiler |

Portability risks: `int` and pointer widths vary; byte order and padding make
raw struct transmission unsafe; fixed-width 32-bit arithmetic can cost more on
small MCUs; and C libraries/compiler support vary by target. The embedded
compiler columns describe common expectations, not a measured build. Confirm
them against the selected board and actual compiler.

## Reusable structures and packet rule

`sample_t` is an in-memory sensor reading; `node_status_t` holds per-node
health/count state. They are API examples, not wire layouts. A packet must
encode each field explicitly, including width, byte order, versioning, and any
scaling, rather than copying a struct's bytes.

## State table

| Current state | Input | Next state |
|---|---|---|
| Any valid state | Null sample, invalid state, or sensor-valid flag absent | `FAULT` |
| `FAULT` | Valid sample below 60.0 | `NORMAL` |
| `FAULT` | Valid sample at or above 60.0 | `FAULT` |
| Any other valid state | Value at or above 100.0 | `FAULT` |
| Any other valid state | Value at or above 80.0 and below 100.0 | `WARNING` |
| Any other valid state | Value below 80.0 | `NORMAL` |

