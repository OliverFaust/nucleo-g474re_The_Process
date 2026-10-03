# CSP4CMSIS 2.0.1 update: results

**Date:** 2026-10-03. **Board:** NUCLEO-G474RE (ST-LINK-V3, VCP = LPUART1, 115200). **Tools:** STM32CubeIDE
2.1.0 (GNU Tools for STM32 14.3.1, headless build), STM32CubeMX 6.17.0, FW_G4 V1.6.3 (FreeRTOS 10.3.1),
STM32CubeProgrammer (flash; SWD reads in hotplug mode, no reset). **Library:** CSP4CMSIS v2.0.1,
unmodified. Baseline (before): `BASELINE.md`. Measurement script: `measure.py`.

## Commits (branch csp4cmsis-2.0.1)

| Commit | Change |
|---|---|
| Prepare for regeneration | heap size (30 720, unchanged) and `USE_NEWLIB_REENTRANT` in the `.ioc`; bootstrap lines into `USER CODE BSP` |
| Migrate to FW_G4 V1.6.3 | 34 HAL/CMSIS/BSP files; `Middlewares/` (FreeRTOS 10.3.1) unchanged |
| CubeMX: static defaultTask; configASSERT | `defaultTask` static (CubeMX re-adds a default task if it is removed); printing `configASSERT` |
| CSP4CMSIS 2.0.1 | library, project settings (Debug and Release), `application.cpp` on CMSIS-RTOS2 |
| Untrack language.settings.xml | gitignored; CubeIDE recreates it on import |
| README | versions, measured memory, corrections |

## Board results

| | Baseline (old library) | 2.0.1 Debug | 2.0.1 Release |
|---|---|---|---|
| Build | Debug only (Release: 15 errors) | 0 errors, 0 warnings | 0 errors, 0 warnings |
| text / data / bss (B) | 34 456 / 132 / 37 804 | 35 964 / 132 / 48 608 | 21 824 / 112 / 48 520 |
| UART (20 s, from reset) | Welcome, bootstrap banner, `--- Single Hello World Process ---`, 21 × `Hello world` | **identical** | **identical** |
| HelloProcess stack (1 KB) | 340 B used | 348 B used | 300 B used |
| MainApp stack (8 KB; 2 KB since the follow-up below) | (heap; not measured) | 348 B used (static) | 300 B used |
| defaultTask stack (2 KB) | (heap; not measured) | 128 B used (static) | 100 B used |
| Priorities Hello / MainApp / defaultTask | 2 / 3 / 24 | 8 / 16 / 24 (read from the TCBs) | 8 / 16 / 24 |
| FreeRTOS heap | 10 472 B peak | **0 allocations** | **0 allocations** |
| newlib `_sbrk` | (not measured) | 1032 B | 1032 B |

- **Start order** (instrumented copies, not committed; a counter recorded in `MainApp` before it exits and
  in `HelloProcess::run()` on entry): baseline MainApp exit 1, Hello 2; 2.0.1 MainApp 1, Hello 2.
  Control (network priority `osPriorityAboveNormal`, above MainApp): Hello 1, MainApp 2. The output is
  the same in all three: the start order is not visible on the console.
- **newlib heap:** with `setvbuf(stdout, NULL, _IONBF, 0)` before the first `printf` (scratch build)
  `_sbrk` is never called, so the 1032 B are `printf`'s `stdout` buffer.
- **The 2.0.1 Debug log** begins with 157 lines from the image that was running before (ST-LINK VCP
  buffer, flushed when the port opens); the run starts at `Welcome to STM32 world !`.

## Regeneration and fresh clone

- **GENERATE CODE** (CubeMX 6.17.0) on the committed `.ioc`: no change in git (tracked files and
  `.mxproject` byte-identical). Rebuilt ELFs byte-identical to those built before the regeneration (Debug
  `7b87d9e2…`, Release `47ce7fd2…`); board output and measurements identical (`regen_debug_uart.txt`).
- **Fresh clone** to another path, imported into an empty workspace, built (Debug and Release, 0
  errors, 0 warnings), flashed: Release ELF byte-identical to the original checkout's (`47ce7fd2…`);
  Debug: flash image identical (`objcopy -O binary`); the ELF contains the build path in its debug
  information. Output and measurements identical
  (`fresh_debug_uart.txt`). `language.settings.xml` was recreated by the import (ignored by git).

## Follow-up: smaller FreeRTOS heap and MainApp stack

`configTOTAL_HEAP_SIZE` 30 720 -> 1024 B (in the `.ioc`; nothing allocates from the FreeRTOS heap: 0
allocations measured; 1 KB still fits one 128-word dynamic thread: 512 B stack + 168 B TCB + heap_4
block headers). MainApp's stack 8 KB -> 2 KB (512 words; measured use 348 B Debug, 300 B Release).

| | Debug | Release |
|---|---|---|
| Build | 0 errors, 0 warnings | 0 errors, 0 warnings |
| text / data / bss (B) | 35 964 / 132 / 12 768 | 21 824 / 112 / 12 680 |
| UART (20 s, from reset) | identical (21 × `Hello world`) | identical |
| Stacks used: Hello / MainApp / defaultTask | 348 / 348 of 2048 / 128 B | 300 / 300 of 2048 / 100 B |
| Priorities | 8 / 16 / 24 | 8 / 16 / 24 |
| FreeRTOS heap / newlib `_sbrk` | 0 allocations / 1032 B | 0 allocations / 1032 B |

Regeneration from the `.ioc` afterwards: no change in git; rebuilt ELFs identical (`small_*` logs).

Logs: `results/` (UART logs with ELF SHA-256; `*_swd.txt`: SWD readings).
