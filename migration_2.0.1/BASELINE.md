# Baseline before the CSP4CMSIS 2.0.1 update

**Date:** 2026-10-03. **Commit:** `a2c948b` (main). **Library:** `lib/csp4cmsis/`, a FreeRTOS-native
pre-1.0 snapshot ("API 1.3", 2026-07-29, same tree as nucleo-g474re_Interrupts and
_Processes_and_Channels); not a CSP4CMSIS release (8 of 28 files equal v1.0.0).
**Build:** STM32CubeIDE 2.1.0 headless (GNU Tools for STM32 14.3.1), Debug (`-O0 -g3`, GNU++17).
Release does not build (no `lib/csp4cmsis/inc` include path in Release: 15 errors).
**Board:** NUCLEO-G474RE, ST-LINK-V3 VCP (LPUART1) at 115200, flashed with STM32CubeProgrammer.

| | Debug |
|---|---|
| ELF SHA-256 | `24f1e6fe8e7c53fa611132bd8f3f7681220a327ad5d0bea3e39ab250388926f2` |
| text / data / bss | 34 456 / 132 / 37 804 B |
| UART (20 s) | banner, `--- Single Hello World Process ---`, `Hello world` once per second (21 lines): `baseline_uart.txt` |
| HelloProcess stack (`CSProcessStatic<256>`, 1024 B) | 85 words (340 B) used, 171 words (684 B) never touched (0xA5 fill, read over SWD after 20 s) |
| FreeRTOS heap (heap_4, 30 720 B) | 28 552 B free now, 20 248 B minimum ever free: 10 472 B peak, mostly MainApp's 8 KB `xTaskCreate` stack (freed by `vTaskDelete`) and defaultTask's 2 KB |

CubeMX regeneration (6.17.0, `.ioc` unchanged except `USE_NEWLIB_REENTRANT` to pass its warning dialog):
removes `csp_app_main_init()` and the bootstrap `printf` from `main.c` (outside USER CODE), and
resets `configTOTAL_HEAP_SIZE` to 3072 (set by hand in `FreeRTOSConfig.h`, not in the `.ioc`).
