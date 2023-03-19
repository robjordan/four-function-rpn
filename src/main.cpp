#include <Arduino.h>
#include <TM1638plus_Model2.h>
#include "fsm.h"

// Set LED_BUILTIN if it is not defined by Arduino framework
#define LED_BUILTIN PIN_PA7

// GPIO I/O pins on the ATTINY connected to strobe, clock, data.
#define  STROBE_TM PIN_PA1 // strobe = GPIO connected to strobe line of module
#define  CLOCK_TM PIN_PA2  // clock = GPIO connected to clock line of module
#define  DIO_TM PIN_PA3 // data = GPIO connected to data line of module
#define TM_SETUP_DELAY 250
bool swap_nibbles = false; //Default is false if left out, see note in readme at URL
bool high_freq = false; //default false,, If using a high freq CPU > ~100 MHZ set to true. 


// Constructor object
TM1638plus_Model2 tm(STROBE_TM, CLOCK_TM , DIO_TM, swap_nibbles, high_freq);

void tm1638_setup();
void tm1638_loop();
void calc_display(void);

void setup()
{
  float f = 123.456;
  double d = 123.456;
  char buf[32];
  delay(1000);
  Serial.begin(9600,SERIAL_8N1);
  Serial.println("setup()");

  tm.displayBegin(); // Init display / keys module
  delay(TM_SETUP_DELAY);
  tm.reset();

  pinMode(LED_BUILTIN, OUTPUT);
  init_stack();

  // // Display digits strings on the TM1638 LED
  // uint8_t dp = 0;
  // char *tests[] = {
  //   "1.2345678", "12.345678", "123.45678", "1234.5678", "12345.678", 
  //   "123456.78", "1234567.8", "12345678.", "1.234e-10", "-1.23e+10"
  // };
  // while (1) {
  //   for (int i=0; i<sizeof tests / sizeof tests[0]; i++) {
  //     char *digits = convert_stack_item_for_led(tests[i], &dp);
  //     tm.DisplayStr(digits, dp);
  //     delay(1000);
  //   }
  // }
}

void loop()
{
  int inbyte;

  while (!Serial.available())
    ; // do nothing
  inbyte = Serial.read();

  // Got a byte - flash the LED briefly
  digitalWrite(LED_BUILTIN, HIGH);
  delay(10);
  digitalWrite(LED_BUILTIN, LOW);

  process_symbol(inbyte);
  char *si;
  for (int i=4; i>0; i--) {
    si = get_stack_item_as_string(i-1);
    Serial.printf("%d: %s\n", i, si);
    Serial.flush();
  }

  // Display stack[0] on the TM1638 LED
  uint8_t dp = 0;
  char *digits = convert_stack_item_for_led(si, &dp);
  tm.DisplayStr(digits, dp);

}