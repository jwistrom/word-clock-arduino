// RTClib - Version: 1.2.2
#include <RTClib.h>
#include <Wire.h>


RTC_DS3231 rtc;

const int minuteButton = 9;
const int hourButton = 8;

const int latchPin = 10;
const int clockPin = 12;
const int dataPin = 13;

void setup() {
  Serial.begin(9600);
  pinMode(minuteButton, INPUT);
  pinMode(hourButton, INPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  //Wire.begin();
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  rtc.adjust(DateTime(2019, 7, 7, 12, 0, 0));
}

int lastChangeMinute = -1;
int lastChangeHour = -1;

void loop() {
  int minuteButtonState = digitalRead(minuteButton);
  int hourButtonState = digitalRead(hourButton);
  if (minuteButtonState == HIGH) {
    Serial.println("Increased minute");
    DateTime plusMinuteTime = rtc.now() + TimeSpan(0, 0, 1, 0);
    rtc.adjust(plusMinuteTime);
    delay(200);
  } else if (hourButtonState == HIGH) {
    Serial.println("Increased hour");
    DateTime plusHourTime = rtc.now() + TimeSpan(0, 1, 0, 0);
    rtc.adjust(plusHourTime);
    delay(200);
  } else {
    DateTime now = rtc.now();
    unsigned int m = now.minute();
    unsigned int h = now.hour();
    if (lastChangeMinute != m || lastChangeHour != h) {
      Serial.println((String)"Time is " + now.hour() + ":" + m);
      changeTime();
      lastChangeMinute = m;
      lastChangeHour = h;
    }
  }

}

void changeTime() {
  DateTime now = rtc.now();
  unsigned int m = now.minute();
  unsigned int h = now.hour();

  byte reg1 = 0x0;
  byte reg2 = 0x0;
  byte reg3 = 0x0;

  h = h % 12; //compensate for 24h representation

  if (m >= 25) { //display next hour
    if (h == 0) {
      h = 1;
    } else {
      h = h + 1;
    }
  }

  if (h < 9 && h > 0) {
    reg1 = 0x1;
    reg1 = reg1 << (h - 1);
  } else if (h > 0) {
    reg2 = 0x1;
    reg2 = reg2 << (h - 9);
  } else { //it is 12 (h==0)
    reg2 = 0x8;
  }

  if (m >= 5 && m < 10) {
    reg3 = 0x41;
  } else if (m >= 10 && m < 15) {
    reg3 = 0x42;
  } else if (m >= 15 && m < 20) {
    reg3 = 0x44;
  } else if (m >= 20 && m < 25) {
    reg3 = 0x48;
  } else if (m >= 25 && m < 30) {
    reg3 = 0x31;
  } else if (m >= 30 && m < 35) {
    reg3 = 0x10;
  } else if (m >= 35 && m < 40) {
    reg3 = 0x51;
  } else if (m >= 40 && m < 45) {
    reg3 = 0x28;
  } else if (m >= 45 && m < 50) {
    reg3 = 0x24;
  } else if (m >= 50 && m < 55) {
    reg3 = 0x22;
  } else if (m >= 55) {
    reg3 = 0x21;
  }

  int onePositionOfMinute = getOnePosition(m) % 5;
  switch (onePositionOfMinute) {
    case 4:
      reg2 = reg2 ^ 0x80;
    case 3:
      reg2 = reg2 ^ 0x40;
    case 2:
      reg2 = reg2 ^ 0x20;
    case 1:
      reg2 = reg2 ^ 0x10;
  }


  //ground latchPin and hold low for as long as you are transmitting
  digitalWrite(latchPin, 0);
  toClock(reg3);
  toClock(reg2);
  toClock(reg1);
  //return the latch pin high to signal chip that it
  //no longer needs to listen for information
  digitalWrite(latchPin, 1);
}

void toClock(byte registerContent) {
  shiftOut(dataPin, clockPin, MSBFIRST, registerContent);
}

unsigned getOnePosition(unsigned value) {
  return value % 10;
}






