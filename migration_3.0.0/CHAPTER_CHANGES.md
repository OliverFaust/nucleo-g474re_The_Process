# What changes in the book chapter text (The Process), 2.0.1 -> 3.0.0

The 2.0.1 list (`../migration_2.0.1/CHECKLIST.md`, end) still applies; for 3.0.0 change only these points:

1. **`application.cpp` listing:** `SleepFor(Milliseconds(1000));` replaces
   `SleepFor(Milliseconds(1000).to_ticks());`. In 3.0 `SleepFor()` takes a duration (`Time`): a plain
   number no longer compiles, because whether it meant ticks or milliseconds would be a guess. Ticks, where
   they are meant: `SleepFor(Ticks(n))`. Everything else in the listing is unchanged (`osThreadNew` launcher,
   `osDelay(10)`, `osThreadExit()`, the priorities, `Run(InParallel(hello), ExecutionMode::StaticNetwork,
   NETWORK_PRIORITY)`: `Run()` must always name its `ExecutionMode` in 3.0, which this listing already does).
2. **Project setup:** two defines instead of four, in both configurations:
   `CSP4CMSIS_MAX_SYSCALL_INTERRUPT_PRIORITY=5` and `CSP4CMSIS_DEVICE_HEADER="stm32g4xx.h"`. CSP4CMSIS 3.0
   allocates its RTOS objects statically by default and finds FreeRTOS from `FreeRTOS.h`; with FreeRTOS this
   needs `configSUPPORT_STATIC_ALLOCATION 1`, which CubeMX's CMSIS_V2 interface sets. A shifted value such as
   0x50 for the priority define does not compile.
3. **Library version:** `lib/csp4cmsis/` is CSP4CMSIS 3.0.0, unmodified (`VERSION`); the 3.x API is stable.
   `csp4cmsis.h` includes `FreeRTOS.h` here (static allocation embeds FreeRTOS control blocks in the process
   objects); the listing keeps its own `#include "FreeRTOS.h"` because MainApp's `StaticTask_t` is its own.
4. **Measured stack use:** HelloProcess 332 B of 1 KB in Debug (was 348 B), 300 B in Release.
5. **Unchanged:** console output, priorities, start order, memory (0 FreeRTOS heap allocations, newlib's
   1 KB `stdout` buffer), CubeMX settings, the formal model.
