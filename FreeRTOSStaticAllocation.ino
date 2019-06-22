#include <MapleFreeRTOS.h>

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

xSemaphoreHandle xSemaphore;
xSemaphoreHandle xMutex;
xQueueHandle xQueue;

void vTask1(void *)
{
  while(1)
  {
    vTaskDelay(1000);
    xSemaphoreGive(xSemaphore);

    xSemaphoreTake(xMutex, portMAX_DELAY);
    Serial.println("Test");
    xSemaphoreGive(xMutex);
  }
}

void vTask2(void *)
{
  while(1)
  {
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    int value = random(1000);
    xQueueSend(xQueue, &value, portMAX_DELAY);
  }
}

void vTask3(void *)
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
  Serial.begin(9600);

  pinMode(PC6, OUTPUT);
  digitalWrite(PC6, 0);

  vSemaphoreCreateBinary(xSemaphore);
  xQueue = xQueueCreate(1000, sizeof(int));
  xMutex = xSemaphoreCreateMutex();

  xTaskCreate(vLEDThread, (const signed char*)"LED Thread", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  xTaskCreate(vTask1, (const signed char*)"Task 1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  xTaskCreate(vTask2, (const signed char*)"Task 2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  xTaskCreate(vTask3, (const signed char*)"Task 3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  vTaskStartScheduler();
}

void loop() {}



