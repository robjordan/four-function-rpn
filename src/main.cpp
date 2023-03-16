#include <Arduino.h>
#include "fsm.h"

// Set LED_BUILTIN if it is not defined by Arduino framework
#define LED_BUILTIN PIN_PA7

void setup()
{
  float f = 123.456;
  double d = 123.456;
  char buf[32];
  delay(1000);
  Serial.begin(9600,SERIAL_8N1);
  Serial.println("setup()");

  // // Unit test float_to_string()
  // Serial.println("Input: 3.14159");
  // float_to_string(3.14159f, buf);
  // Serial.println(buf);  

  // Serial.println("Input: 31459");
  // float_to_string((float)31459, buf);
  // Serial.println(buf);  

  // Serial.println("Input: 12345678");
  // float_to_string((float)12345678, buf);
  // Serial.println(buf); 

  // Serial.println("Input: 123456789");
  // float_to_string((float)123456789, buf);
  // Serial.println(buf);  

  // Serial.println("Input: 3.14159e5");
  // float_to_string(3.14159e5f, buf);
  // Serial.println(buf);  

  // Serial.println("Input: 3.14159e10");
  // float_to_string(3.14159e10f, buf);
  // Serial.println(buf);  

  // Serial.println("Input: 3.14159e-5");
  // float_to_string(3.14159e-5f, buf);
  // Serial.println(buf);  

  // Serial.println("Input: 3.14159e-15");
  // float_to_string(3.14159e-15f, buf);
  // Serial.println(buf);  

  // // Negative numbers
  // Serial.println("Input: -3.14159");
  // float_to_string(-3.14159f, buf);
  // Serial.println(buf);  

  // Serial.println("Input: -31459");
  // float_to_string((float)-31459, buf);
  // Serial.println(buf);  

  // Serial.println("Input: -123456789");
  // float_to_string((float)-123456789, buf);
  // Serial.println(buf);  

  // Serial.println("Input: -3.14159e5");
  // float_to_string(-3.14159e5f, buf);
  // Serial.println(buf);  

  // Serial.println("Input: -3.14159e10");
  // float_to_string(-3.14159e10f, buf);
  // Serial.println(buf);  

  // Serial.println("Input: -3.14159e-5");
  // float_to_string(-3.14159e-5f, buf);
  // Serial.println(buf);  

  // Serial.println("Input: -3.14159e-15");
  // float_to_string(-3.14159e-15f, buf);
  // Serial.println(buf); 

  // Serial.println("Input: pow(10.0, (float)DISPDIGITS))"); 
  // float_to_string(pow(10.0, (float)DISPDIGITS), buf);
  // Serial.println(buf);

  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  init_stack();
}

void loop()
{
  int inbyte;

  while (!Serial.available())
    ; // do nothing
  inbyte = Serial.read();
  process_symbol(inbyte);
  for (int i=4; i>0; i--) {
    Serial.print(i);
    Serial.print(": ");
    Serial.println(get_stack_item_as_string(i-1));
  }

  // Got a byte - flash the LED briefly
  digitalWrite(LED_BUILTIN, HIGH);
  delay(10);
  digitalWrite(LED_BUILTIN, LOW);
  // Echo inbyte to output
}