# Checklist: updating a book example to CSP4CMSIS 2.0.1

Done first on nucleo-g474re_The_Process (`RESULTS.md`), then on nucleo-g474re_Processes_and_Channels
(its `migration_2.0.1/RESULTS.md`; generic scripts `measure.py`, `seqcheck.py`, `run.sh` there).
Remaining: _Interrupts, _Alternation, _Sensor_Data_Processing_Network.
Work on a branch `csp4cmsis-2.0.1`; commit locally; one commit per step below.

Tools: STM32CubeIDE 2.1.0, STM32CubeMX 6.17.0, FW_G4 V1.6.3, STM32CubeProgrammer, the CSP4CMSIS
repository at tag v2.0.1 (for the library and `tests/hw_nucleo_g474/nucleo_run.py`), NUCLEO-G474RE.
Build headless: `headless-build.sh -data <empty workspace> -import <repo> -cleanBuild <project>`.
Regenerate headless: CubeMX `-q` script `config load <ioc>` / `project generate` / `exit`
(a "Warning: Code Generation" dialog means `USE_NEWLIB_REENTRANT` is still off).

## 1. Analysis and baseline (commit: `migration_2.0.1/BASELINE.md`)

- [ ] `git fetch`; fast-forward the default branch; branch off. Note the checked-out branch (e.g.
      _Alternation is on `develop`).
- [ ] Library: `git rev-parse HEAD:lib/csp4cmsis` and compare with the other repositories and CSP4CMSIS
      history (_Interrupts and _Processes_and_Channels carry the same pre-1.0 snapshot as this one).
- [ ] `.ioc`: `MxCube.Version`, `FirmwarePackage`, `VP_FREERTOS_VS_CMSIS_V2` (all five: CMSIS_V2),
      `FREERTOS.*` (heap size and newlib set in the `.ioc` or by hand in `FreeRTOSConfig.h`?).
- [ ] `.cproject`: include paths and defines per configuration; language standard per configuration
      (Release had no GNU++17 here).
- [ ] `main.c` and `FreeRTOSConfig.h`: application code outside `USER CODE` markers (regeneration
      deletes it); hand edits to generated lines (`configTOTAL_HEAP_SIZE`).
- [ ] API inventory of `Core/Src/application.cpp`: FreeRTOS calls, priorities, stack sizes,
      `putFromISR`, ALT, channels (table in section 5).
- [ ] Every channel: type (`Channel<T>` = `SamplingChannel<T, Block>` = rendezvous in the old snapshot
      and in 2.0.1; `BufferedChannel<T, N>`), policy, element type, writers/readers, ISR use.
      **Stop and report before changing anything** if the example uses a policy other than Block on
      a rendezvous channel, `putFromISR` (on any channel), a non-trivially-copyable element type,
      ALT with timeouts, or anything else whose meaning changes in 2.0: those compile differently or
      not at all in 2.0.1 (see the table in section 5).
- [ ] Top-level `LICENSE`: The_Process and _Processes_and_Channels said "Copyright (c) 2024 Your Name".
- [ ] Build Debug and Release; flash Debug; log the UART (`nucleo_run.py`, 20 s); read the stack fill
      (0xA5) of each process and thread, the TCB priorities, the heap_4 counters
      (`xFreeBytesRemaining`, `xMinimumEverFreeBytesRemaining`, `xNumberOfSuccessfulAllocations`,
      `xNumberOfSuccessfulFrees`) and `__sbrk_heap_end` over SWD in hotplug mode
      (_Processes_and_Channels' `measure.py --stack/--tcb`). Process objects (`CSProcessStatic<N>`):
      stack at +0x10, TCB at +0x10 + 4·N, in the old snapshot and in 2.0.1 (check with
      `arm-none-eabi-gdb`: `print &((Class*)0)->m_stack`).
- [ ] A baseline with **0 frees** after the launcher's `vTaskDelete(NULL)` is not a code leak: FreeRTOS
      frees a deleted task's memory in the idle task, which never runs while the network is always
      ready (e.g. a receiver printing continuously).
- [ ] Continuous output (channel examples): compare the header and the value sequence
      (`seqcheck.py`: in order, no gaps), not the number of lines in the log window.

## 2. Make regeneration safe (commit)

- [ ] `.ioc`: `FREERTOS.configTOTAL_HEAP_SIZE=<current value>`, `FREERTOS.configUSE_NEWLIB_REENTRANT=1`,
      both names added to `FREERTOS.IPParameters`.
- [ ] Move every application line in `main.c` into a `USER CODE` section (here: `USER CODE BSP`,
      which runs after the console init and before `osKernelStart()`). Mind CRLF line endings.
- [ ] Regenerate with the current firmware; the diff must be only the intended lines, `.cproject`
      reordering and `.mxproject` (commit it: relative paths only). Do not commit
      `.settings/language.settings.xml`.

## 3. Migrate to FW_G4 V1.6.3 (own commit)

- [ ] `.ioc`: `ProjectManager.FirmwarePackage=STM32Cube FW_G4 V1.6.3`, `MxCube.Version=6.17.0`,
      `MxDb.Version=DB.6.0.170`; regenerate.
- [ ] Check: `git diff --stat Middlewares` empty (FreeRTOS 10.3.1 in both V1.6.1 and V1.6.3); only HAL,
      CMSIS device and BSP files change. Debug builds. (_Sensor_Data_Processing_Network is already
      on V1.6.3 / 6.17.0: skip.)

## 4. CubeMX: static defaultTask, printing configASSERT (commit)

- [ ] `FREERTOS.Tasks01=defaultTask,24,<words>,StartDefaultTask,Default,NULL,Static,defaultTaskBuffer,defaultTaskControlBlock`
      (CubeMX re-adds a default task if `Tasks01` is removed, with 128 words). Regenerate.
- [ ] `FreeRTOSConfig.h` `USER CODE 1`: `void vAssertCalled(const char *file, int line);` and
      `#define configASSERT( x ) if ((x) == 0) { vAssertCalled(__FILE__, __LINE__); }`;
      `main.c`: `#include <stdio.h>` (`USER CODE Includes`), `vAssertCalled()` (`USER CODE 4`).

## 5. Port to 2.0.1 (commit)

- [ ] `git rm -r lib/csp4cmsis`; copy `git archive v2.0.1 csp4cmsis LICENSE` output: `csp4cmsis/` to
      `lib/csp4cmsis/`, `LICENSE` to `lib/csp4cmsis/LICENSE`; write `lib/csp4cmsis/VERSION`
      (`CSP4CMSIS 2.0.1`, tag, commit `6c23fec`, "unmodified"). `diff -r` against the archive:
      only `LICENSE` and `VERSION` extra.
- [ ] `.cproject`, G++ compiler, **Debug and Release**: include path `../lib/csp4cmsis/inc` only;
      defines `CSP4CMSIS_RTOS2_BACKEND_FREERTOS`, `CSP4CMSIS_MAX_SYSCALL_INTERRUPT_PRIORITY=5`
      (= `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY`), `CSP4CMSIS_STATIC_ALLOCATION`,
      `CSP4CMSIS_DEVICE_HEADER="stm32g4xx.h"` (XML: `&quot;`); language standard GNU++17 in both.
      Source folder `lib/csp4cmsis/src` stays.
- [ ] Application (2.0.1's `csp4cmsis.h` no longer includes `FreeRTOS.h`/`task.h`):

  | Old | 2.0.1 | Note |
  |---|---|---|
  | `vTaskDelay(pdMS_TO_TICKS(ms))` in a process | `SleepFor(Milliseconds(ms).to_ticks())` | `SleepFor` takes ticks, `Time` has an explicit constructor |
  | `vTaskDelay(...)` in the launcher | `osDelay(ticks)` | 1 kHz tick: ms = ticks |
  | `xTaskCreate(f, name, words, arg, prio, NULL)` | `osThreadNew(f, arg, &attr)`, static `stack_mem`/`cb_mem` | **`stack_size` in bytes** (`sizeof` the buffer); `alignas(8)` stack; `StaticTask_t` needs `FreeRTOS.h`; failure is `NULL`, not `pdPASS` |
  | `vTaskDelete(NULL)` | `osThreadExit()` | |
  | `tskIDLE_PRIORITY + n`, `UBaseType_t` priorities | named `osPriority_t` | ST's wrapper passes the value unchanged to FreeRTOS |
  | `Run(InParallel(...), mode)` | `Run(InParallel(...), mode, priority)` | default priority was native 2, is now `osPriorityLow` (8): pass it explicitly, chosen against the launcher's priority (start order) |
  | `CSProcessStatic<N>` | same | N in words (1 word = 4 B) |
  | `taskPriority()` / `stackWords()` overrides | `osPriority_t taskPriority()`; `stackWords()` is final in `CSProcessStatic` | |
  | `chan.writer().putFromISR(x)` (_Interrupts, _Sensor) | `SamplingBufferedChannel<T, SIZE>` + `chan.isrWriter().putFromISR(x)` | **semantics**: an ISR can write only into a buffered channel (a rendezvous `Channel<T>` has no ISR writer); `sizeof(T) <= 64` (compile-time check); ISR priority numerically >= 5; no `portYIELD_FROM_ISR` needed (the wakeup, `osThreadFlagsSet`/`osSemaphoreRelease`, yields in ISR context on ST's wrapper) |
  | `Alternative alt(in | var, ...)`, `fairSelect()` (_Alternation) | same syntax in 2.0.1 | check guard and channel types compile; timeouts: `RelTimeoutGuard` (2.0.1: no RTOS timer) |

- [ ] Channels (2.0.1, checked at compile time): rendezvous `Channel<T>` needs a trivially copyable `T`,
      accepts only `BufferPolicy::Block`, and has no ISR writer; it creates no RTOS object (critical
      section + thread flags). Buffered channels use CMSIS-RTOS2 semaphores (`csp_semaphore.h`),
      static with `CSP4CMSIS_STATIC_ALLOCATION`.
- [ ] Priorities: state the intended order in a code comment; check on the board (TCB `uxPriority`)
      and, where it matters, the start order with an instrumented scratch copy plus a control with the
      order reversed. The old network ran at native 2, the same as CubeMX's timer task (2); at
      `osPriorityLow` (8) it runs above it (no software timers are used).
- [ ] Right-size from the board measurements (first with a generous MainApp stack, e.g. 2 KB):
      MainApp stack about twice the Debug use, with the measured Debug/Release values in the comment;
      `FREERTOS.configTOTAL_HEAP_SIZE=1024` in the `.ioc` when 0 FreeRTOS heap allocations are measured
      (1 KB still fits one 128-word dynamic thread), with a comment in `FreeRTOSConfig.h`
      `USER CODE Defines` (generated lines cannot hold comments).
- [ ] Build Debug and Release: 0 errors, 0 warnings.

## 6. Untrack `.settings/language.settings.xml`; fix LICENSE (one commit each)

- [ ] `git rm --cached`; add to `.gitignore`. (Verified by the fresh-clone import in step 8.)
- [ ] `LICENSE`: "Copyright (c) <year of the first commit> Oliver Faust"; MIT text unchanged.

## 7. README (commit)

- [ ] Tested versions table; CSP4CMSIS settings; regeneration note; project structure
      (`lib/csp4cmsis/`, not a submodule); console = LPUART1 (ST-LINK VCP); formal-model link to the
      repository's own folder; memory claims **only as measured** (FreeRTOS heap allocations, newlib
      `_sbrk`, stacks); How it works / Troubleshooting with the 2.0.1 calls.
- [ ] Describe printed output exactly (e.g. _Processes_and_Channels' receiver prints the received
      value twice as `Send: X Received: X`).

## 8. Verification (commit `RESULTS.md` and logs)

- [ ] Board (Debug and Release): output from reset identical to the baseline; stacks, priorities,
      FreeRTOS heap (allocation count), `_sbrk` (a per-example `run.sh` calling the generic
      `measure.py` with `--stack`/`--tcb` for each process and thread). The first lines of a log can
      be stale VCP data from the previously running image: compare from the reset banner.
- [ ] Regenerate from the `.ioc`: `git status` clean; rebuilt ELFs byte-identical (then the board
      output is identical by construction).
- [ ] Fresh clone to another path, empty workspace, import, build both configurations, flash:
      output identical; Release ELF identical.

---

## What changes in the book chapter text (The Process)

1. **`application.cpp` listing:**
   - `SleepFor(Milliseconds(1000).to_ticks())` replaces `vTaskDelay(pdMS_TO_TICKS(1000))`.
   - The launcher uses `osThreadNew` with an attribute structure (static stack and control
     block), `osDelay(10)` and `osThreadExit()`.
   - New includes: `cmsis_os2.h`, and `FreeRTOS.h` for `StaticTask_t`.
   - Two named priority constants with the start-order comment; `Run()` gets the priority as a third
     argument.
2. **Stack units:** `CSProcessStatic<256>` counts words (1 KB), while `osThreadAttr_t.stack_size`
   counts bytes (MainApp: 512 words = 2 KB, measured use 348 B; the old `xTaskCreate(..., 2048, ...)`
   counted words, i.e. 8 KB).
3. **Priorities:**
   - CMSIS-RTOS2 names and values: HelloProcess `osPriorityLow` (8), MainApp `osPriorityBelowNormal`
     (16), defaultTask `osPriorityNormal` (24); formerly 2, 3 and 24.
   - The default composition priority of `Run()` changed from native 2 to `osPriorityLow`.
   - Why the launcher runs above the network: HelloProcess first runs after MainApp has exited.
4. **Start-up:**
   - `csp_app_main_init()` is called inside `USER CODE BSP` in `main.c`, and why the markers matter
     (CubeMX regeneration deletes code outside them).
5. **CubeMX settings** the reader makes or sees:
   - `USE_NEWLIB_REENTRANT` Enabled;
   - heap size set in the `.ioc`: 1024 B (was 30 KB by hand; nothing allocates from it now);
   - `defaultTask` Allocation Static (CubeMX does not let you remove it);
   - the printing `configASSERT`;
   - FW_G4 V1.6.3, CubeMX 6.17.0, CubeIDE 2.1.0.
6. **Project setup:**
   - The library is `lib/csp4cmsis/`: CSP4CMSIS 2.0.1, unmodified, with `VERSION` and `LICENSE`.
   - Include path `../lib/csp4cmsis/inc`.
   - The four defines, in both configurations; GNU++17 in both.
   - CSP4CMSIS is a CMSIS-RTOS2 library: `csp4cmsis.h` no longer brings in FreeRTOS headers.
7. **Memory claims:**
   - Not "zero heap" but: no FreeRTOS heap allocation (measured: 0); newlib's `printf` allocates a
     1 KB `stdout` buffer.
   - Measured stack use: HelloProcess 348 B of 1 KB (Debug).
8. **Console:** LPUART1 via the ST-LINK virtual COM port, not USART1.
9. **Unchanged:** the console output, the `HelloProcess` class (except the sleep call),
   `InParallel`, `ExecutionMode::StaticNetwork`, and the formal model.
