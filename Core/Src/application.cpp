#include <cstdio>
#include "csp/csp4cmsis.h"

using namespace csp;

class HelloProcess : public CSProcessStatic<256> {
 public:
  const char* name() const override { return "HelloProcess"; }

  void run() override {
    while (true) {
      printf("Hello world\r\n");
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
};

void MainApp_Task(void* params) {
  vTaskDelay(pdMS_TO_TICKS(10));
  printf("\r\n--- Single Hello World Process ---\r\n");

  static HelloProcess hello;

  Run(InParallel(hello), ExecutionMode::StaticNetwork);

  vTaskDelete(NULL);
}

void csp_app_main_init(void) {
  BaseType_t status = xTaskCreate(MainApp_Task, "MainApp", 2048, NULL, tskIDLE_PRIORITY + 3, NULL);
  if (status != pdPASS) {
    printf("ERROR: MainApp_Task creation failed!\r\n");
  }
}
