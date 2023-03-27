#include <Arduino.h>
#include <TM1638plus_Model2.h>
#include "fsm.h"

// Set LED_BUILTIN if it is not defined by Arduino framework
#define LED_BUILTIN PIN_PA7

// GPIO I/O pins on the ATTINY connected to strobe, clock, data.
#define  STROBE_TM PIN_PB1 // strobe = GPIO connected to strobe line of module
#define  CLOCK_TM PIN_PA2  // clock = GPIO connected to clock line of module
#define  DIO_TM PIN_PA3 // data = GPIO connected to data line of module
#define TM_SETUP_DELAY 250
#define TICK_MS 10
#define SLEEP_TICKS (6000) // Number of idle ticks before screen is turned off
bool swap_nibbles = false; //Default is false if left out, see note in readme at URL
bool high_freq = false; //default false,, If using a high freq CPU > ~100 MHZ set to true. 
unsigned long idle_time = 0;
bool idle = false;


char symbol[] = {
  '0',
  // short press key mapping
  '7', '8', '9', '/',
  '4', '5', '6', '*',
  '1', '2', '3', '-', 
  '0', '.', 0x0a, '+',

  // long press key mapping
  '7', '8', '9', '/',
  '4', '5', '6', '*',
  '1', '2', '3', 'C', 
  '0', '.', 0x0a, '+',  

};


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

  // To save power, make sure unuused pins aren't floating
  pinMode(PIN_PA4, INPUT_PULLUP);
  pinMode(PIN_PA5, INPUT_PULLUP);
  pinMode(PIN_PA6, INPUT_PULLUP);
  pinMode(PIN_PB3, INPUT_PULLUP);
  pinMode(PIN_PB2, INPUT_PULLUP);
  pinMode(PIN_PA1, INPUT_PULLUP);
  pinMode(PIN_PA0, INPUT_PULLUP);
  pinMode(PIN_PB0, INPUT_PULLUP);

  // Ability to flash the LED if needed
  pinMode(LED_BUILTIN, OUTPUT);
  VPORTA.OUT &= ~PIN7_bm;

  tm.displayBegin(); // Init display / keys module
  delay(TM_SETUP_DELAY);
  tm.reset();

  pinMode(LED_BUILTIN, OUTPUT);
  init_stack();

  tm.DisplayStr("       0", 0);
}

void loop()
{
  uint8_t button;
  int hold_time = 0;

  VPORTA.IN = PIN7_bm; // toggle the output

  button = tm.ReadKey16();

  if (button > 0 && button <= 16) {
    while (tm.ReadKey16() != 0) {
      // Wait for key release
      delay(TICK_MS);
      hold_time += TICK_MS;
    }

    // The long-press option
    if (hold_time > 500)
      button += 16;

    // Ignore the wake-up key 
    if (idle) {
      // wake up
      idle = false;
      idle_time = 0;
    } else {
      // process the keystroke
      process_symbol(symbol[button]);
    }

    char *si;
    for (int i=4; i>0; i--) {
      si = get_stack_item_as_string(i-1);
      Serial.printf("%d: %s\n", i, si);
      // Serial.flush();

    }

    // Display stack[0] on the TM1638 LED
    uint8_t dp = 0;
    char *digits = convert_stack_item_for_led(si, &dp);
    tm.DisplayStr(digits, dp);
    idle_time = 0;

  }

  delay(TICK_MS);
  idle_time++;
  if ((idle_time > SLEEP_TICKS) && idle == false) {
    tm.reset();
    idle = true;
  }

}