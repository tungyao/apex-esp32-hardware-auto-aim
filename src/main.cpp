#include <Arduino.h>

#include "USB.h"
#include "USBHIDMouse.h"

USBHIDMouse mouse;

// ####### FREERTOS
#define MAX_LINE_LENGTH (2) // expassion

// Define two tasks for reading and writing from and to the serial port.
void TaskMouseMove(void *pvParameters);
void TaskReadFromSerial(void *pvParameters);

QueueHandle_t QueueHandle;
const int QueueElementSize = 10;

typedef struct
{
  char move[MAX_LINE_LENGTH];
} message_t;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }
  mouse.begin();
  USB.serialNumber("5201314");
  USB.manufacturerName("apex tools");
  USB.productName("this is apex auto aim aux");
  USB.begin();

  // init queue
  QueueHandle = xQueueCreate(QueueElementSize, sizeof(message_t));

  if (QueueHandle == NULL)
  {
    Serial.println("Queue could not be created. Halt.");
    while (1)
    {
      delay(1000); 
    }
  }
  xTaskCreatePinnedToCore(TaskMouseMove,"move listen",4096 ,NULL,1,NULL,0);
  xTaskCreatePinnedToCore(TaskReadFromSerial,"Task Read From Serial",4096,NULL,1,NULL,1);
}

void loop()
{
  if (Serial.available() > 0)
  {
    char buffer[2];
    Serial.readBytesUntil('\n', buffer, 2);
    int8_t x = buffer[0];
    int8_t y = buffer[1];
    mouse.move(x, y);
  }
}

void TaskMouseMove(void *pvParameters)
{
  message_t message;
  for (;;)
  {
    if (QueueHandle != NULL)
    { // Sanity check just to make sure the queue actually exists
      int ret = xQueueReceive(QueueHandle, &message, portMAX_DELAY);
      if (ret == pdPASS)
      {
        mouse.move(message.move[0], message.move[1]);
      }
    }
  }
}

void TaskReadFromSerial(void *pvParameters)
{
  message_t message;
  for (;;)
  {
    // Check if any data are waiting in the Serial buffer
    if (Serial.available() > 0)
    {
      // Check if the queue exists AND if there is any free space in the queue
      if (QueueHandle != NULL && uxQueueSpacesAvailable(QueueHandle) > 0)
      {

        char buffer[2];
        Serial.readBytesUntil('\n', buffer, 2);
        message.move[0] = buffer[0];
        message.move[1] = buffer[1];

        int ret = xQueueSend(QueueHandle, (void *)&message, 0);
        if (ret == pdTRUE)
        {
          // The message was successfully sent.
        }
        else if (ret == errQUEUE_FULL)
        {
          Serial.println("The `TaskReadFromSerial` was unable to send data into the Queue");
        }
      }
    }
  }
}