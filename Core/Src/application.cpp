#include <cstdio>
#include "cmsis_os2.h"
#include "FreeRTOS.h"  // StaticTask_t: the control block of a statically created thread
#include "csp/csp4cmsis.h"

using namespace csp;

class HelloProcess : public CSProcessStatic<256> {  // stack: 256 words = 1 KB
 public:
  const char* name() const override { return "HelloProcess"; }

  void run() override {
    while (true) {
      printf("Hello world\r\n");
      SleepFor(Milliseconds(1000).to_ticks());
    }
  }
};

// Start order. MainApp runs at a higher priority than the network it launches, so
// Run(..., StaticNetwork) only creates HelloProcess's thread and returns: HelloProcess
// cannot preempt MainApp and first runs after MainApp has printed its banner and exited.
// Both stay below CubeMX's defaultTask (osPriorityNormal), as before.
static constexpr osPriority_t MAIN_APP_PRIORITY = osPriorityBelowNormal;
static constexpr osPriority_t NETWORK_PRIORITY  = osPriorityLow;

// MainApp's stack and control block are static: creating the thread takes no heap.
// CMSIS-RTOS2 counts the stack in bytes: 2048 words = 8 KB.
alignas(8) static uint32_t mainAppStack[2048];
static StaticTask_t mainAppControlBlock;

void MainApp_Task(void* argument) {
  (void)argument;
  osDelay(10);
  printf("\r\n--- Single Hello World Process ---\r\n");

  static HelloProcess hello;

  Run(InParallel(hello), ExecutionMode::StaticNetwork, NETWORK_PRIORITY);

  osThreadExit();
}

void csp_app_main_init(void) {
  osThreadAttr_t attr = {};
  attr.name       = "MainApp";
  attr.stack_mem  = mainAppStack;
  attr.stack_size = sizeof(mainAppStack);
  attr.cb_mem     = &mainAppControlBlock;
  attr.cb_size    = sizeof(mainAppControlBlock);
  attr.priority   = MAIN_APP_PRIORITY;
  if (osThreadNew(MainApp_Task, NULL, &attr) == NULL) {
    printf("ERROR: MainApp_Task creation failed!\r\n");
  }
}
