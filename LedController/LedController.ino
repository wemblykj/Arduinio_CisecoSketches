//////////////////////////////////////////////////////////////////////////
// RFµ PIR Sensor
//
//
//
// Uses the Ciseco LLAPSerial library
//
// https://code.google.com/p/tinkerit/wiki/SecretVoltmeter
//////////////////////////////////////////////////////////////////////////

#include <EEPROM.h>
#include <LLAPSerial.h>

#define VERSION "0.1"

#define DEVICETYPE "LED"
#define DEVICEID1 'L'
#define DEVICEID2 'C'
#define EEPROM_DEVICEID1 0
#define EEPROM_DEVICEID2 1
#define EEPROM_IDLETIME 2
#define EEPROM_OVERRUNTIME 6

#define SWITCH_PIN 2
#define BUTTON_PIN 3
#define R_PIN 5
#define G_PIN 6
#define B_PIN 9

#define WAKEC 10
byte battc = 9;

#define IDLETIME 50               // time in ms to block after a trigger
#define OVERRUNTIME 2000          // time to overrun before switching lights off

bool debug = false;
int idleTime = IDLETIME;
int overrunTime = OVERRUNTIME;

String msg;        // storage for incoming message
String reply;    // storage for reply

void setup() {
    pinMode(8, OUTPUT);             // pin 8 controls the radio
    digitalWrite(8, HIGH);          // select the radio

    Serial.begin(115200);

    pinMode(4, OUTPUT);             // pin 4 controls the radio sleep
    digitalWrite(4, LOW);           // wake the radio

    delay(450);                     // allow the radio to startup

    pinMode(SWITCH_PIN, INPUT_PULLUP);              // switch input pin
    //pinMode(SWITCH_PIN, INPUT);              // switch input pin
    //digitalWrite(SWITCH_PIN, HIGH);           // pullup

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    pinMode(R_PIN, OUTPUT);
    pinMode(G_PIN, OUTPUT);
    pinMode(B_PIN, OUTPUT);

    // Default to on
    digitalWrite(R_PIN, HIGH);
    digitalWrite(G_PIN, HIGH);
    digitalWrite(B_PIN, HIGH);
  
    String permittedChars = "-#@?\\*ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char deviceID[2];
    deviceID[0] = EEPROM.read(EEPROM_DEVICEID1);
    deviceID[1] = EEPROM.read(EEPROM_DEVICEID2);
  
    if (permittedChars.indexOf(deviceID[0]) == -1 || permittedChars.indexOf(deviceID[1]) == -1)
    {
      deviceID[0] = DEVICEID1;
      deviceID[1] = DEVICEID2;
    }     

    EEPROM.get(EEPROM_IDLETIME, idleTime);
    if (idleTime == -1)
    {
      idleTime = IDLETIME;
    }
    
    EEPROM.get(EEPROM_OVERRUNTIME, overrunTime);
    if (overrunTime == -1)
    {
      overrunTime = OVERRUNTIME;
    }
    
    LLAP.init(deviceID);
    
    LLAP.sendMessage(F("STARTED"));

    debug = digitalRead(BUTTON_PIN) == LOW;
    
    if (debug)
    {
      LLAP.sendMessage(String("IDLE")+idleTime);
      LLAP.sendMessage(String("ORUN")+overrunTime);
    }
}

void loop() {
    if (LLAP.bMsgReceived)
    {
      digitalWrite(R_PIN, LOW);
      digitalWrite(G_PIN, HIGH);
      digitalWrite(B_PIN, LOW);
      
      msg = LLAP.sMessage;
      LLAP.bMsgReceived = false;
      reply = msg;
      if (msg.compareTo("HELLO----") == 0)
      {
          ;    // just echo the message back
      }
      else if (msg.compareTo("FVER-----") == 0)
      {
        reply = reply.substring(0,4) + VERSION;
      }
      else if (msg.compareTo("DEVTYPE--") == 0)
      {
        reply = DEVICETYPE;
      }
      else if (msg.compareTo("SAVE-----") == 0)
      {
          EEPROM.write(EEPROM_DEVICEID1, LLAP.deviceId[0]);    // save the device ID
          EEPROM.write(EEPROM_DEVICEID2, LLAP.deviceId[1]);    // save the device ID
          EEPROM.put(EEPROM_IDLETIME, idleTime); 
          EEPROM.put(EEPROM_OVERRUNTIME, overrunTime); 
      }
      else if (msg.compareTo("BATT-----") == 0)
      {
        LLAP.sendIntWithDP("BATT", int(readVcc()),3);    // read the battery voltage and send
      }
      else if (msg.startsWith("IDLE") == 0)
      {
        msg = msg.substring(4);
        if (msg.startsWith("-"))
        {    // read the value
            reply = reply.substring(0,4) + idleTime;
        }
        else
        {    // set the value
            int value = msg.toInt();
            if (value >=0 && value <= 99999)
                idleTime = value;
            else
                reply = "TOOLARGE";
        }
      }
      else if (msg.startsWith("ORUN") == 0)
      {
        msg = msg.substring(4);
        if (msg.startsWith("-"))
        {    // read the value
            reply = reply.substring(0,4) + overrunTime;
        }
        else
        {    // set the value
            int value = msg.toInt();
            if (value >=0 && value <= 99999)
                overrunTime = value;
            else
                reply = "TOOLARGE";
        }
      }
      else
      {
        digitalWrite(R_PIN,HIGH);
        digitalWrite(G_PIN, LOW);
        digitalWrite(B_PIN, LOW);
        reply = "ERROR";
      }

      LLAP.sendMessage(reply);
      
      //LLAP.sleepForaWhile(1000);
      
      digitalWrite(R_PIN, HIGH);
      digitalWrite(G_PIN, HIGH);
      digitalWrite(B_PIN, HIGH);
    }
    else if (digitalRead(SWITCH_PIN) == LOW)
    {
      LLAP.sleepForaWhile(overrunTime);
      
      // Switch off lights
      digitalWrite(R_PIN, LOW);
      digitalWrite(G_PIN, LOW);
      digitalWrite(B_PIN, LOW);

      LLAP.sendMessage(F("OFF"));                  // the switch triggered send a message

      delay(450);  
      
      pinMode(4, INPUT);                          // sleep the radio
    
			if (digitalRead(SWITCH_PIN) == LOW)					// double check we should still sleep after overrun
				LLAP.sleep(SWITCH_PIN, RISING, true);     // deep sleep until SWITCH causes interupt, with pullup

      // WAKE UP ///

      pinMode(4, OUTPUT);                         // wake the radio
      
      // Switch on lights
      digitalWrite(R_PIN, HIGH);
      digitalWrite(G_PIN, HIGH);
      digitalWrite(B_PIN, HIGH);
      
      battc++;                                    // increase battery count
  
      delay(450);                                 // give it time to wake up
  
      LLAP.sendMessage(F("ON"));                  // the switch triggered send a message
      
      if (battc >= WAKEC) {                       // is it time to send a battery reading
          battc = 0;
          LLAP.sendIntWithDP("BATT", int(readVcc()),3);    // read the battery voltage and send
      }
    }
    else if (!debug)
    {
      debug = digitalRead(BUTTON_PIN) == LOW;
      //LLAP.sendMessage(F("IDLE"));
      LLAP.sleepForaWhile(idleTime);           // sleep for a little while before we go back to listening for messages
    }
      
}

long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}
