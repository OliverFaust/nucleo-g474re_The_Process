# Book notes

The source files deliberately contain no comments: the explanations are in the book, which refers to the code by line. This file keeps every comment removed from this chapter's code on 2026-10-12 (from commit `bdb3dad`), so that nothing is lost.

For each file, in file order: where the comment was (line numbers before the removal), the comment itself, and the code it explains, numbered as in the source file now, i.e. as in the book listing.

## `Core/Src/application.cpp`

1. **Line 3**, at the end of the code line; explains listing line 3:

   ```cpp
   // StaticTask_t
   ```

   ```cpp
   3  #include "FreeRTOS.h"
   ```

2. **Line 8**, at the end of the code line; explains listing line 8:

   ```cpp
   // stack: 256 words = 1 KB
   ```

   ```cpp
   8  class HelloProcess : public CSProcessStatic<256> {
   ```

3. **Lines 18–19**, above the code; explains listing lines 18–19:

   ```cpp
   // MainApp runs above the network, so HelloProcess first runs after MainApp has printed
   // its banner and exited.
   ```

   ```cpp
   18  static constexpr osPriority_t MAIN_APP_PRIORITY = osPriorityBelowNormal;
   19  static constexpr osPriority_t NETWORK_PRIORITY  = osPriorityLow;
   ```

4. **Line 23**, above the code; explains listing lines 21–22:

   ```cpp
   // Static stack (512 words = 2 KB) and control block: no heap.
   ```

   ```cpp
   21  alignas(8) static uint32_t mainAppStack[512];
   22  static StaticTask_t mainAppControlBlock;
   ```
