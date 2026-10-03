# CSP4CMSIS Hello World Demo for NUCLEO-G474RE

A minimal demonstration of the **CSP (Communicating Sequential Processes)** library CSP4CMSIS using CMSIS‑RTOS v2 on an STM32G474RE microcontroller. This project shows a single process that prints "Hello world" repeatedly – the simplest possible Communicating Sequential Processes (CSP) network. The corresponding formal CSP model is in [`Formal model/`](Formal%20model/).

## Features

- **FreeRTOS** with the CMSIS‑RTOS v2 API (STM32CubeMX `CMSIS_V2` interface)
- **CSP4CMSIS 2.0.1** library for process‑based concurrency
- **No FreeRTOS heap allocation**: every thread's stack and control block is static (see [Memory](#memory))
- **Serial console output** via LPUART1, the ST‑LINK virtual COM port (115200 baud)
- **Infinite loop** with a 1‑second delay to avoid console flooding

## Hardware Requirements

- STM32 Nucleo‑G474RE board  
- USB cable for power, programming, and serial communication  
- No external components required

## Software Requirements

Tested with:

| Tool | Version |
|---|---|
| STM32CubeIDE | 2.1.0 (GNU Tools for STM32 14.3.rel1) |
| STM32CubeMX (only to regenerate code) | 6.17.0 |
| STM32Cube FW_G4 | V1.6.3 (FreeRTOS 10.3.1) |
| CSP4CMSIS | 2.0.1, in `lib/csp4cmsis/` (unmodified; see `lib/csp4cmsis/VERSION`) |

## Serial Configuration

| Parameter   | Value          |
|-------------|----------------|
| Baud Rate   | 115200         |
| Data Bits   | 8              |
| Stop Bits   | 1              |
| Parity      | None           |
| Flow Control| None           |

## Building with STM32CubeIDE

1. **Clone this repository** (do not place it inside your STM32CubeIDE workspace directory).  
2. Open STM32CubeIDE.  
3. Go to `File → Import → Existing Projects into Workspace`.  
4. Select the cloned directory.  
5. Build the project (configuration `Debug` or `Release`).  
6. Flash the binary to your Nucleo board.

The CSP4CMSIS settings are already in the project (G++ compiler, Debug and Release): include path `../lib/csp4cmsis/inc`, and the defines `CSP4CMSIS_RTOS2_BACKEND_FREERTOS`, `CSP4CMSIS_MAX_SYSCALL_INTERRUPT_PRIORITY=5`, `CSP4CMSIS_STATIC_ALLOCATION` and `CSP4CMSIS_DEVICE_HEADER="stm32g4xx.h"` (explained in the [CSP4CMSIS STM32CubeIDE guide](https://github.com/OliverFaust/CSP4CMSIS/blob/main/Documentation/CSP4CMSIS_STM32CubeIDE.md)).

## Regenerating code with STM32CubeMX

`nucleo-g474re_v10.ioc` can be opened and regenerated (GENERATE CODE) without losing anything: the application's code in `main.c` and `FreeRTOSConfig.h` sits between `USER CODE BEGIN`/`END` markers, and the FreeRTOS settings it needs (heap size, newlib reentrancy, static default task) are stored in the `.ioc`.

## Project Structure
```text
├── Core/            # main.c (CubeMX), application.cpp (the example)
├── Drivers/         # STM32 HAL, CMSIS and BSP drivers
├── Formal model/    # CSP-M model of HelloProcess
├── lib/csp4cmsis/   # CSP4CMSIS 2.0.1 (inc/, src/, LICENSE, VERSION)
├── Middlewares/     # FreeRTOS + CMSIS‑RTOS v2
├── nucleo-g474re_v10.ioc  # STM32CubeMX project
└── README.md
```


## How It Works

1. **Process**: `HelloProcess` inherits from `CSProcessStatic<256>`, a CSP process with a static 256‑word (1 KB) stack, and overrides the `run()` method.  
2. **Infinite loop**: Inside `run()`, the process prints `"Hello world"` and then sleeps for 1000 ms with `SleepFor(Milliseconds(1000).to_ticks())`.  
3. **Parallel composition**: `InParallel(hello)` composes the single process.  
4. **Static network**: `Run(..., ExecutionMode::StaticNetwork, priority)` creates the process's thread and returns; the network runs for ever.  
5. **Start-up**: `main.c` calls `csp_app_main_init()`, which creates the `MainApp` thread (static 8 KB stack). `MainApp` prints the banner, starts the network and exits. `MainApp` runs at a higher priority (`osPriorityBelowNormal`) than the network (`osPriorityLow`), so `HelloProcess` first runs after `MainApp` has exited.

Because there is only one process, no channels are needed – the process runs independently.

## Example Console Output

```text
Welcome to STM32 world !

=== STM32 FreeRTOS + CSP4CMSIS bootstrap ===

--- Single Hello World Process ---
Hello world
Hello world
Hello world
...
```

## Memory

Measured on the board (Debug and Release, after 20 s):

- **FreeRTOS heap: not used.** `pvPortMalloc()` is never called (0 allocations). `HelloProcess`, `MainApp`, CubeMX's `defaultTask`, and FreeRTOS's idle and timer tasks all have static stacks and control blocks. The 30 KB FreeRTOS heap (`configTOTAL_HEAP_SIZE`) stays reserved but unused.
- **C library heap: 1 KB.** newlib's `printf()` allocates its `stdout` buffer with `malloc()` on first use (1032 B from `_sbrk()`). This is the only dynamic allocation.
- **Stacks used** (Debug; Release in brackets): `HelloProcess` 348 B (300 B) of 1 KB, `MainApp` 348 B (300 B) of 8 KB, `defaultTask` 128 B (100 B) of 2 KB.

## Key CSP4CMSIS Concepts Demonstrated
* **Process** – creating a custom process by inheriting from `CSProcessStatic<N>`.
* **Run** – the process entry point.
* **InParallel** – composing one or more processes.
* **Static network** – a network whose processes are created once at start-up and run for ever, with static stacks and control blocks.
* **CSP scheduling** – processes are threads, scheduled by the RTOS at the priority given to `Run()`.

## Troubleshooting
* **No output on serial**: Verify the baud rate (115200) and that the correct COM port (the ST‑LINK virtual COM port) is used.
* **Program hangs**: Ensure `SleepFor()` is called in the loop; without it the process never blocks, and threads at lower priority (the FreeRTOS idle and timer tasks) never run.
* **`configASSERT failed: <file>:<line>`** on the console: a FreeRTOS assertion failed at that source line; the program halts there.

## License

MIT License – see the `LICENSE` file. CSP4CMSIS: MIT License, `lib/csp4cmsis/LICENSE`.

## Acknowledgments

- STMicroelectronics for the STM32 HAL and CMSIS‑RTOS v2
- The FreeRTOS team
- [CSP4CMSIS](https://oliverfaust.github.io/CSP4CMSIS/) library by Oliver Faust
