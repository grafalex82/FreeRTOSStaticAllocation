#include <STM32FreeRTOS.h>

template<class T, size_t size>
class Queue
{
  QueueHandle_t xHandle;
  StaticQueue_t xQueueDefinition;
  T             xStorage[size];

public:
  Queue()
  {
    xHandle = xQueueCreateStatic(size,
                     sizeof(T),
                     reinterpret_cast<uint8_t*>(xStorage),
                     &xQueueDefinition);
  }

  bool receive(T * val, TickType_t xTicksToWait = portMAX_DELAY)
  {
    return xQueueReceive(xHandle, val, xTicksToWait);
  }

  bool send(const T & val, TickType_t xTicksToWait = portMAX_DELAY)
  {
    return xQueueSend(xHandle, &val, xTicksToWait);
  }
};

class Sema
{
  SemaphoreHandle_t xSema;
  StaticSemaphore_t xSemaControlBlock;

public:
  Sema()
  {
    xSema = xSemaphoreCreateBinaryStatic(&xSemaControlBlock);
  }

  BaseType_t give()
  {
    return xSemaphoreGive(xSema);
  }
  
  BaseType_t take(TickType_t xTicksToWait = portMAX_DELAY)
  {
    return xSemaphoreTake(xSema, xTicksToWait);
  }
};

class Mutex
{
  SemaphoreHandle_t xMutex;
  StaticSemaphore_t xMutexControlBlock;

public:
  Mutex()
  {
    xMutex = xSemaphoreCreateMutexStatic(&xMutexControlBlock);
  }

  BaseType_t lock(TickType_t xTicksToWait = portMAX_DELAY)
  {
    return xSemaphoreTake(xMutex, xTicksToWait);
  }
  
  BaseType_t unlock()
  {
    return xSemaphoreGive(xMutex);
  }  
};

class MutexLocker
{
  Mutex & mtx;
  
public:
  MutexLocker(Mutex & mutex)
    : mtx(mutex)
  {
    mtx.lock();
  }
  
  ~MutexLocker()
  {
    mtx.unlock();
  }
};

class Timer
{
  TimerHandle_t xTimer;
  StaticTimer_t xTimerControlBlock;

public:
  Timer(const char * const pcTimerName,
        const TickType_t xTimerPeriodInTicks,
        const UBaseType_t uxAutoReload,
        void * const pvTimerID,
        TimerCallbackFunction_t pxCallbackFunction)
    {
      xTimer = xTimerCreateStatic(pcTimerName, xTimerPeriodInTicks, uxAutoReload, pvTimerID, pxCallbackFunction, &xTimerControlBlock);
    }

    void start(TickType_t xTicksToWait = 0)
    {
      xTimerStart(xTimer, xTicksToWait);
    }
};

template<const uint32_t ulStackDepth>
class Task
{
protected:
  StaticTask_t xTaskControlBlock;
  StackType_t xStack[ ulStackDepth ];
  TaskHandle_t xTask;

public:
  Task(TaskFunction_t pxTaskCode,
       const char * const pcName,
       void * const pvParameters,
       UBaseType_t uxPriority)
  {
    xTask = xTaskCreateStatic(pxTaskCode, pcName, ulStackDepth, pvParameters, uxPriority, xStack, &xTaskControlBlock);
  }
};


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

Timer xTimer("Timer", 1000, pdTRUE, NULL, vTimerCallback);
Sema xSema;
Mutex xMutex;
Queue<int, 1000> xQueue;

Task<configMINIMAL_STACK_SIZE> ledTask(vLEDThread, "LED Thread", NULL, tskIDLE_PRIORITY);
Task<configMINIMAL_STACK_SIZE> task1(vTask1, "Task 1", NULL, tskIDLE_PRIORITY);
Task<configMINIMAL_STACK_SIZE> task2(vTask2, "Task 2", NULL, tskIDLE_PRIORITY);


void vTimerCallback(TimerHandle_t pxTimer)
{
  xSema.give();
  
  MutexLocker lock(xMutex);
  Serial.println("Test");
}

void vTask1(void *)
{
  while(1)
  {
    xSema.take();
    int value = random(1000);
    xQueue.send(value);
  }
}

void vTask2(void *)
{
  while(1)
  {
    int value;
    xQueue.receive(&value);

    MutexLocker lock(xMutex);
    Serial.println(value);
  }
}

void setup()
{
  pinMode(PC6, OUTPUT);
  digitalWrite(PC6, 0);

  Serial.begin(9600);

  xTimer.start();
  
  vTaskStartScheduler();
}

void loop() {}

extern "C" void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
  static StaticTask_t Idle_TCB;
  static StackType_t  Idle_Stack[configMINIMAL_STACK_SIZE];

  *ppxIdleTaskTCBBuffer = &Idle_TCB;
  *ppxIdleTaskStackBuffer = Idle_Stack;
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

extern "C" void vApplicationGetTimerTaskMemory (StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
  static StaticTask_t Timer_TCB;
  static StackType_t  Timer_Stack[configTIMER_TASK_STACK_DEPTH];
  
  *ppxTimerTaskTCBBuffer   = &Timer_TCB;
  *ppxTimerTaskStackBuffer = Timer_Stack;
  *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;
}

