#include "RTClib.h"
RTC_PCF8523 rtc;
#include <Adafruit_NeoPixel.h>
#define PIN        2
#define NUMPIXELS 132
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
#include <SoftwareSerial.h>
#endif
#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
//#define MODE_LED_BEHAVIOUR          "MODE"
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);
extern uint8_t packetbuffer[];

uint8_t red = packetbuffer[2];
uint8_t green = packetbuffer[3];
uint8_t blue = packetbuffer[4];

void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

//Contains the LED values in each row of the clock, on the LED board they are numbered in a snake pattern
int leds[11][12] = {
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},                           // ----->
  {12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23},                 // <-----
  {24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35},                 // ----->
  {36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47},                 // <-----
  {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59},                 // ----->
  {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71},                 // <-----
  {72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83},                 // ----->
  {84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95},                 // <-----
  {96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107},         // ----->
  {108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119},     // <-----
  {120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131}      // ----->
};

//Contains the LED values for the minutes
int mins[12][2] = {
  { -1, -1},         //Empty value for when it's just an hour
  {30, 33},          //Five
  {38, 40},          //Ten
  {15, 21},          //Fifteen
  {24, 29},          //Twenty
  {24, 33},          //Twentyfive
  {42, 47},          //Thirty
  {24, 33},          //Twentyfive
  {24, 29},          //Twenty
  {15, 21},          //Fifteen
  {38, 40},          //Ten
  {30, 33}           //Five
};

//Contains the LED values for the hours
int hrs[24][2] = {
  {78, 83},          //Twelve
  {109, 111},        //One
  {84, 86},          //Two
  {112, 116},        //Three
  {61, 64},          //Four
  {88, 91},          //Five
  {117, 119},        //Six
  {72, 76},          //Seven
  {96, 100},         //Eight
  {92, 95},          //Nine
  {120, 122},        //Ten
  {102, 107},        //Eleven
  {78, 83},          //Twelve
  {109, 111},        //One
  {84, 86},          //Two
  {112, 116},        //Three
  {61, 64},          //Four
  {88, 91},          //Five
  {117, 119},        //Six
  {72, 76},          //Seven
  {96, 100},         //Eight
  {92, 95},          //Nine
  {120, 122},        //Ten
  {102, 107}         //Eleven
};

//Contains the LED values for the extra words
int extras[6][2] = {
  {0, 1},            //It
  {3, 4},            //Is
  {126, 131},        //O'clock
  {48, 54},          //Minutes
  {68, 71},          //Past
  {67, 68}           //To
};

//Contains a set of values for every five minutes
int intv[] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60};

void setup() {                                           //Runs once when the code is initiated
  Serial.begin(115200);
  Serial.println(F("Clock Thing Test"));
  Serial.println(F("-----------------------------------------"));
  Serial.print(F("Initialising the Bluefruit LE module: "));
  if ( !ble.begin(VERBOSE_MODE) ) {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE ) {
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  ble.info();
  ble.verbose(false);
  Serial.println(F("******************************"));
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    //  Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    // ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }
  ble.setMode(BLUEFRUIT_MODE_DATA);

  //Serial.begin(9600);                                     //Starts the serial monitor
  if (! rtc.begin()) {                                    //For when the clock fails to start
    Serial.flush();                                        //Sends all of the backed up data
    abort();                                               //Stops the program
  }
  if (! rtc.initialized() || rtc.lostPower()) {           //For when the clock is loaded or loses and regains power
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));        //Syncs the clock's time with computer's time
    //Serial.print(1);
  }
  rtc.start();                                            //Starts clock
  pixels.begin();                                         //Sets up the LEDs
  pixels.clear();                                         //Turns off all LEDs
  pixels.setPixelColor(20, pixels.Color(200, 200, 200));
  pixels.show();
}

//int r = 0;           //Sets a variable for the red value
//int g = 0;           //Sets a variable for the green value
//int b = 0;           //Sets a variable for the blue value

//Changes minutes
void change_minute(int this_minute, int this_second) {                     //Defines the function and adds values that can be passed into the function
  for (int i = mins[this_minute][0]; i <= mins[this_minute][1]; i++) {      //Sets up LEDs for the corresponding five minute interval
    pixels.setPixelColor(i, pixels.Color(red, green, blue));                        //Sets the color of the LEDs
  }
}

//Changes hours
void change_hour(int this_hour, int this_minute, int this_second) {             //Defines the function and adds values that can be passed into the function
  if (this_minute < intv[7]) {                                                   //For when it's less than 35 minutes into the hour
    for (int i = hrs[this_hour][0]; i <= hrs[this_hour][1]; i++) {                //Sets up the LEDs for the corresponding hour
      pixels.setPixelColor(i, pixels.Color(red, green, blue));                            //Sets the color of the LEDs
    }
  } else {                                                                       //For when it's greater than 35 minutes into the hour
    for (int i = hrs[this_hour + 1][0]; i <= hrs[this_hour + 1][1]; i++) {        //Sets up the LEDs for the corresponding hour
      pixels.setPixelColor(i, pixels.Color(red, green, blue));                            //Sets the color of the LEDs
    }
  }
}

//Changes extra words
void change_extras(int this_minute) {                        //Defines the function and adds values that can be passed into the function
  int extra_words;                                            //Sets a variable for "extra_words"
  if (this_minute >= 5) {                                     //For when the time is not on an hour (greater than or equal to 5 minutes into the hour)
    if (this_minute < intv[7]) {                               //For when it's less than 35 minutes into the hour
      extra_words = 4;                                          //Sets up the LEDs for when the words "It", "Is", "Minutes", "Past", and "O'clock" should show
    } else {                                                   //For when it's not less than 35 minutes into the hour
      extra_words = 3;                                          //Sets up the LEDs for when the words "It", "Is", "Minutes", and "O'clock" should show
    }
  } else {                                                    //For when the time is on an hour (less than 5 minutes into the hour)
    extra_words = 2;                                           //Sets up for the LEDs for when the words "It", "Is", and "O'clock" should show
  }
  for (int i = 0; i <= extra_words; i++) {                    //Sets up the LEDs for the corresponding number of extra words that need to appear
    for (int j = extras[i][0]; j <= extras[i][1]; j++) {       //Sets up LEDs for the corresponding extra words
      pixels.setPixelColor(j, pixels.Color(red, green, blue));         //Sets the color of the LEDs
    }
  }
  if (this_minute >= intv[7]) {                               //For when it's less than 35 minutes into the hour
    for (int i = extras[5][0]; i <= extras[5][1]; i++) {       //Sets up the LEDs for when the word "To" should show
      pixels.setPixelColor(i, pixels.Color(red, green, blue));         //Sets the color of the LEDs
    }
  }
}

//Lights up LEDs
void show_pixels() {                              //Defines the function
  DateTime now = rtc.now();                        //Allows for "now." functions to be called
  Serial.print(1);
  int time_sec = now.second();                     //Sets a variable to the current second
  int time_min = now.minute();                     //Sets a variable to the current minute
  int time_hr = now.hour();                        //Sets a variable to the current hour
  if ((time_min % 5) == 0) {                       //For when the time is on a five minute interval
    //r = 255;                                        //Sets the red value
    //g = 0;                                          //Sets the green value
    //b = 0;                                          //Sets the blue value
    pixels.clear();                                 //Turns off all of the LEDs
    change_minute((time_min / 5), time_sec);        //Calls the minute changinng function and passes the current time into it
    change_hour(time_hr, time_min, time_sec);       //Calls the hour changing function and passes the current time into it
    change_extras(time_min);                        //Calls the extra word changing function and passes the current time into it
    pixels.show();                                  //Turns on the LEDs that were called for in the previous three functions
  }
}

void loop() {        //Executes repeatedly
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len == 0) return;
  if (packetbuffer[1] == 'C') {
    red = packetbuffer[2];
    green = packetbuffer[3];
    blue = packetbuffer[4];
  }
  show_pixels();      //Lights up LEDs
  Serial.println(red);
}

/*
  array [red, green, blue]
  if (arrayred != red || arraygreen != green || arrayblue != blue) {
  reset array [red, green, blue]
  pixels.clear();
  change_minute((time_min / 5), time_sec);
  change_hour(time_hr, time_min, time_sec);
  change_extras(time_min);
  pixels.show();
  }

  variables for hr min and sec
  send through BLEUart thing
  take those and set them to the hr min and sec variables
  count from there (need to chnage the thing that sets the time based on the computer)
         rtc.adjust(DateTime(F(__DATE__), F(hr, min, sec))); <--- maybe? might need to set date also to make it work*/
