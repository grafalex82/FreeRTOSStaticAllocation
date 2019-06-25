#include <MapleFreeRTOS821.h>

void vLEDThread(void *)
{
  pinMode(PB14, OUTPUT_OPEN_DRAIN);
  
  while(1)
  {
    digitalWrite(PB14, true);
    vTaskDelay(1000);
    digitalWrite(PB14, false);
    vTaskDelay(100);
  }
}

TimerHandle_t xTimer;
xSemaphoreHandle xSemaphore;
xSemaphoreHandle xMutex;
xQueueHandle xQueue;

void vTimerCallback(TimerHandle_t pxTimer)
{
  xSemaphoreGive(xSemaphore);
  
  xSemaphoreTake(xMutex, portMAX_DELAY);
  Serial.println("Test");
  xSemaphoreGive(xMutex);
}

void vTask1(void *)
{
  while(1)
  {
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    int value = random(1000);
    xQueueSend(xQueue, &value, portMAX_DELAY);
  }
}

void vTask2(void *)
{
  while(1)
  {
    int value;
    xQueueReceive(xQueue, &value, portMAX_DELAY);

    xSemaphoreTake(xMutex, portMAX_DELAY);
    Serial.println(value);
    xSemaphoreGive(xMutex);
  }
}


void setup()
{
  pinMode(PC6, OUTPUT);
  digitalWrite(PC6, 0);

  Serial.begin(9600);

  delay(1000);

  vSemaphoreCreateBinary(xSemaphore);
  xQueue = xQueueCreate(1000, sizeof(int));
  xMutex = xSemaphoreCreateMutex();

  xTimer = xTimerCreate("Timer", 1000, pdTRUE, NULL, vTimerCallback);
  xTimerStart(xTimer, 0);
  
  xTaskCreate(vLEDThread, "LED Thread", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  xTaskCreate(vTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  xTaskCreate(vTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  vTaskStartScheduler();
}

void loop() {}



