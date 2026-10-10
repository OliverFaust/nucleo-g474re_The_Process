#include <cstdio>
#include "cmsis_os2.h"
#include "FreeRTOS.h"  // StaticTask_t
#include "csp/csp4cmsis.h"

using namespace csp;

class HelloProcess : public CSProcessStatic<256> {  // stack: 256 words = 1 KB
 public:
  void run() override {
    while (true) {
      printf("Hello world\r\n");
      SleepFor(Milliseconds(1000));
    }
  }
};

// MainApp runs above the network, so HelloProcess first runs after MainApp has printed
// its banner and exited.
static constexpr osPriority_t MAIN_APP_PRIORITY = osPriorityBelowNormal;
static constexpr osPriority_t NETWORK_PRIORITY  = osPriorityLow;

// Static stack (512 words = 2 KB) and control block: no heap.
alignas(8) static uint32_t mainAppStack[512];
static StaticTask_t mainAppControlBlock;

void MainApp_Task(void* argument) {
  (void)argument;
  osDelay(10);
  printf("\r\n--- Single Hello World Process (Zero-Heap) ---\r\n");

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
