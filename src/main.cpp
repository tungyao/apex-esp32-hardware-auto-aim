#include <Arduino.h>

#include "USB.h"
#include "USBHIDMouse.h"

USBHIDMouse mouse;

void setup()
{
  Serial.begin(115200);
  mouse.begin();
  USB.serialNumber("5201314");
  USB.manufacturerName("apex tools");
  USB.productName("this is apex auto aim aux");
  USB.begin();
}

void loop()
{
  if (Serial.available() > 0)
  {
    char buffer[4];
    Serial.readBytesUntil('\n', buffer, 4);
  }

  mouse.move(0, 0);
}
