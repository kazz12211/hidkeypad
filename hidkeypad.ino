/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/*
  This example shows how to send HID (keyboard/mouse/etc) data via BLE
  Note that not all devices support BLE keyboard! BLE Keyboard != Bluetooth Keyboard
*/

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#include "Keypad.h"

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {10, 11, 12, 13}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 6, 9}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

char lastChar = 0;
unsigned long tm;


/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         0
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
/*=========================================================================*/


// Create the bluefruit object, either software serial...uncomment these lines
/*
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
*/

/* ...or hardware serial, which does not need the RTS/CTS pins. Uncomment this line */
// Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  //Serial.println(err);
  while (1);
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
  //while (!Serial);  // required for Flora & Micro
  //delay(500);

  //Serial.begin(115200);
  //Serial.println(F("HID Keyboard"));
  //Serial.println(F("---------------------------------------"));

  /* Initialise the module */
  //Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  //Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  //Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  if (! ble.sendCommandCheckOK(F( "AT+GAPDEVNAME=Bike Controller" )) ) {
    error(F("Could not set device name?"));
  }

  /* Enable HID Service */
  //Serial.println(F("Enable HID Service (including Keyboard): "));
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    if ( !ble.sendCommandCheckOK(F( "AT+BleHIDEn=On" ))) {
      error(F("Could not enable Keyboard"));
    }
  }else
  {
    if (! ble.sendCommandCheckOK(F( "AT+BleKeyboardEn=On"  ))) {
      error(F("Could not enable Keyboard"));
    }
  }

  /* Add or remove service requires a reset */
  //Serial.println(F("Performing a SW reset (service changes require a reset): "));
  if (! ble.reset() ) {
    error(F("Couldn't reset??"));
  }

}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
    char key = keypad.getKey();
    if(key != NO_KEY) {
      tm = millis();
      if(key == '#') {
        if(lastChar == '#') {
          lastChar = 0;
        } else {
          lastChar = key;
        }
      } else if(key == '*') {
        if(lastChar == '*') {
          lastChar = 0;
        } else {
          lastChar = key;
        }
      } else {
        if(lastChar == '#') {
          if(key >= '1' && key <= '8' && key != '7') {
            ble.print("AT+BleHidControlKey=");
            switch(key) {
              case '1': //
                ble.println("BRIGHTNESS+");
                break;
              case '2': // volume up
                ble.println("VOLUME+");
                break;
              case '3': //
                ble.println("BRIGHTNESS-");
                break;
              case '4': // previous
                ble.println("MEDIAPREVIOUS");
                break;
              case '5': // play/stop
                ble.println("PLAYPAUSE");
                break;
              case '6': // next
                ble.println("MEDIANEXT");
                break;
              case '8': // volume down
                 ble.println("VOLUME-");
                 break;
              default:
                break;
            }
            if(ble.waitForOK()) {
              //Serial.println(F("OK"));
            } else {
              //Serial.println(F("FAILED"));
            }
          }
        } else {
          char buf[2] = " ";
          int code = key - '0';
          if(lastChar == '*') {
            buf[0] = 'a' + (key - '0');
          } else {
            buf[0] = key;
          }
          ble.print("AT+BleKeyboard=");
          ble.println(buf);
          lastChar = 0;
          if(ble.waitForOK()) {
            //Serial.println(F("OK"));
          } else {
            //Serial.println(F("FAILED"));
          }
        }
     }
      delay(10);
    }
    if(millis() - tm > 2000) {
      tm = millis();
      lastChar = 0;
    }
}


