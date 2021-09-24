#include "RTClib.h"
RTC_PCF8523 rtc;
#include <Adafruit_NeoPixel.h>
#define PIN        6
#define NUMPIXELS 132
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int leds[11][12] = {
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
  {12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23},
  {24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35},
  {36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47},
  {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59},
  {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71},
  {72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83},
  {84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95},
  {96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107},
  {108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119},
  {120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131}
};

int mins[12][2] = {
  { -1, -1},
  {30, 33}, //five
  {38, 40},
  {15, 21},
  {24, 29},
  {24, 33},
  {42, 47}, //thirty
  {24, 33},
  {24, 29},
  {15, 21},
  {38, 40},
  {30, 33} //five
};

int hrs[24][2] = {
  {109, 111}, //one
  {84, 86}, //two
  {112, 116},
  {61, 64},
  {88, 91},
  {117, 119},
  {72, 76},
  {96, 100},
  {92, 95},
  {120, 122},
  {102, 107},
  {78, 83} //twelve
};

int extras[6][2] = {
  {0, 1}, //it
  {3, 4}, //is
  {126, 131}, //o'clock
  {48, 54}, //minutes
  {68, 71}, //past
  {67, 68} //to
};

int intv[] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60};

void setup() {
  Serial.begin(57600);
  if (! rtc.begin()) {
    Serial.flush();
    abort();
  }
  if (! rtc.initialized() || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.start();
  float drift = 43;
  float period_sec = (9600);
  float deviation_ppm = (drift / period_sec * 1000000);
  float drift_unit = 4.34;
  int offset = round(deviation_ppm / drift_unit);
  pixels.begin();
  pixels.clear();
}

void change_minute(int this_minute, int this_second) {
  for (int i = mins[this_minute][0]; i <= mins[this_minute][1]; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 100));
   Serial.print(this_minute);
  }
}

void change_hour(int this_hour, int this_minute, int this_second) {
  if (this_minute < intv[7]) {
    for (int i = hrs[this_hour][0]; i <= hrs[this_hour][1]; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 100));
    }
  } else {
    for (int i = hrs[this_hour + 1][0]; i <= hrs[this_hour + 1][1]; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 100));
    }
  }
}

void change_extras(int this_minute) {
  int extra_words;
  if (this_minute > 4) {
    if (this_minute < intv[7]) {
      extra_words = 4;
    } else {
      extra_words = 3;
    }
  } else {
    extra_words = 2;
  }
  for (int i = 0; i <= extra_words; i++) {
    for (int j = extras[i][0]; j <= extras[i][1]; j++) {
      pixels.setPixelColor(j, pixels.Color(0, 0, 100));
    }
  }
  if (this_minute >= intv[7]) {
    for (int i = extras[5][0]; i <= extras[5][1]; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 100));
    }
  }
}

void show_pixels() {
  DateTime now = rtc.now();
  int time_sec = now.second();
  int time_min = now.second();
  int time_hr = now.minute() % 12;
  if ((time_min % 5) == 0) {
    pixels.clear();
    change_minute((time_min / 5), time_sec);
    //pixels.show();
    change_hour(time_hr, time_min, time_sec);
    //pixels.show();
    //change_extras(time_min);
    pixels.show();
  }
}

void loop() {
  show_pixels();
}
