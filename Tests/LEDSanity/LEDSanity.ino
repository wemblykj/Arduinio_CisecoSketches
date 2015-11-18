/* 
 *  LEDSanity
 *  A small sketch for testing the sanity of LED hardware
 *  Paul Wightmore 2015
 */

const int B_PIN = 5;
const int R_PIN = 6;
const int G_PIN = 9;

void setRGB(uint8_t val)
{
  digitalWrite(B_PIN, val);
  digitalWrite(R_PIN, val);
  digitalWrite(G_PIN, val);
}

void setRGB(uint8_t r_val, uint8_t g_val, uint8_t b_val)
{
  digitalWrite(B_PIN, b_val);
  digitalWrite(R_PIN, r_val);
  digitalWrite(G_PIN, g_val);
}

void setup() {
  // initialize RGB pins as outputs.
  pinMode(B_PIN, OUTPUT);
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
}

void loop() {
  
  setRGB(LOW);              // initialise LEDs off
  delay(1000);              // wait for a second
  setRGB(HIGH);
  delay(1000);              // wait for a second
}
