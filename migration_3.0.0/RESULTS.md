# CSP4CMSIS 3.0.0 update: results

**Date:** 2026-10-06. **Board:** NUCLEO-G474RE (ST-LINK-V3, VCP = LPUART1, 115200). **Tools:** STM32CubeIDE
2.1.0 (GNU Tools for STM32 14.3.1, headless build), STM32CubeMX 6.17.0, FW_G4 V1.6.3 (FreeRTOS 10.3.1),
STM32CubeProgrammer. **Library:** CSP4CMSIS v3.0.0 (commit `647a1cb`), unmodified. Reference: the 2.0.1
results (`../migration_2.0.1/RESULTS.md`, follow-up with the 1 KB heap and 2 KB MainApp stack; logs
`../migration_2.0.1/results/small_*`). Measurement script: `../migration_2.0.1/measure.py` (the process
object layout is unchanged in 3.0: stack at +0x10, control block at +0x410).

## Commits (branch csp4cmsis-3.0.0)

| Commit | Change |
|---|---|
| CSP4CMSIS 3.0.0: lib/csp4cmsis | unmodified v3.0.0 sources, `LICENSE`, `VERSION` |
| Debug and Release defines | only `CSP4CMSIS_MAX_SYSCALL_INTERRUPT_PRIORITY=5` and `CSP4CMSIS_DEVICE_HEADER="stm32g4xx.h"`; the backend and static-allocation defines removed (3.0 detects FreeRTOS from `FreeRTOS.h` and allocates statically by default) |
| `SleepFor(Milliseconds(1000))` | was `SleepFor(Milliseconds(1000).to_ticks())` |
| README | 3.0.0, the two defines, the sleep call |

## Board results

| | 2.0.1 Debug | 3.0.0 Debug | 2.0.1 Release | 3.0.0 Release |
|---|---|---|---|---|
| Build | 0 errors, 0 warnings | 0 errors, 0 warnings | 0 errors, 0 warnings | 0 errors, 0 warnings |
| text / data / bss (B) | 35 964 / 132 / 12 768 | 36 008 / 132 / 12 776 | 21 824 / 112 / 12 680 | 21 736 / 112 / 12 688 |
| UART (20 s, from reset) | banner, `--- Single Hello World Process ---`, 21 × `Hello world` | **identical** | same | **identical** |
| Stacks used: Hello / MainApp / defaultTask | 348 / 348 / 128 B | 332 / 348 / 128 B | 300 / 300 / 100 B | 300 / 300 / 100 B |
| Priorities Hello / MainApp / defaultTask | 8 / 16 / 24 | 8 / 16 / 24 | 8 / 16 / 24 | 8 / 16 / 24 |
| FreeRTOS heap / newlib `_sbrk` | 0 allocations / 1032 B | 0 allocations / 1032 B | 0 allocations / 1032 B | 0 allocations / 1032 B |

- **Static allocation without the define:** 0 FreeRTOS heap allocations, as in 2.0.1 with
  `CSP4CMSIS_STATIC_ALLOCATION`: HelloProcess's thread control block is static (in the process object).
  FreeRTOS was detected from `FreeRTOS.h` (no `RTE_Components.h` in a CubeIDE project).
- **HelloProcess stack, Debug:** 332 B instead of 348 B; Release unchanged (300 B). Likely cause, not
  verified: `SleepFor(Milliseconds(1000))` passes the `Time` on, so the `.to_ticks()` call and its
  temporary, which `-O0` keeps, are gone.
- **Sizes:** Debug text +44 B, Release −88 B, bss +8 B (not examined further). The process object itself
  is unchanged (1456 B for `CSProcessStatic<256>`; the measurement offsets still match).

## Regeneration and fresh clone

- **GENERATE CODE** (CubeMX 6.17.0) on the committed `.ioc`: no change in git. Rebuilt ELFs byte-identical
  (Debug `e59e82c8…`, Release `99725f43…`, as on the board).
- **Fresh clone** to another path, imported into an empty workspace, built (Debug and Release, 0 errors,
  0 warnings): Release ELF byte-identical (`99725f43…`), Debug flash image identical (`objcopy -O binary`;
  the Debug ELF holds the build path). Flashed: output identical (`results/fresh_debug_uart.txt`).

Logs: `results/` (`v300_*_uart.txt` with the ELF SHA-256, `v300_*_swd.txt`: SWD readings).
