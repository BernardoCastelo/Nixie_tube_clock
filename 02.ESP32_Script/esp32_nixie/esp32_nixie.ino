#include <WiFi.h>
#include "time.h"
#include "constsLib.h"

struct tm timeinfo;
volatile byte hours;
volatile byte minutes;
volatile byte seconds;
volatile int millisValue = 0;
volatile int ntpMillisValue = 0;
volatile bool isShufflingSeconds = false;
volatile bool isShufflingMinutes = false;
volatile bool isShufflingHours = false;
volatile bool isConnecting = false;
volatile int tmpCalc;

void setup() {
  //Serial.begin(9600);
  pinMode(OE_H, OUTPUT);
  pinMode(OE_M, OUTPUT);
  pinMode(OE_S, OUTPUT);
  digitalWrite(OE_H, HIGH);
  digitalWrite(OE_M, HIGH);
  digitalWrite(OE_S, HIGH);

  pinMode(CLK_PIN, OUTPUT);
  pinMode(STR_H, OUTPUT);
  pinMode(STR_M, OUTPUT);
  pinMode(STR_S, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(RESET, OUTPUT);

  pinMode(BTN_H, INPUT);
  pinMode(BTN_M, INPUT);
  pinMode(TEMP_PIN, INPUT);

  digitalWrite(CLK_PIN, LOW);
  digitalWrite(STR_H, LOW);
  digitalWrite(STR_M, LOW);
  digitalWrite(STR_S, LOW);
  digitalWrite(DATA_PIN, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }

  hours = timeinfo.tm_hour;
  minutes = timeinfo.tm_min;
  seconds = timeinfo.tm_sec;

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  digitalWrite(OE_H, LOW);
  digitalWrite(OE_M, LOW);
  digitalWrite(OE_S, LOW);
}

void loop() {
  if (isShufflingSeconds) {
    writeDisplay(SECOND, random(0, 99));
    delay(10);
  }

  if (isShufflingMinutes) {
    writeDisplay(MINUTE, random(0, 99));
    delay(10);
  }

  if (isShufflingHours) {
    writeDisplay(HOUR, random(0, 99));
    delay(10);
  }

  /*
     Synchronous (dumb) time counter and minute/hour calculator,
     as is it deviates about 1 minute for every 6H.
  */
  if (millis() - millisValue > 1000) {
    millisValue = millis();
    seconds++;
    shuffle();
    if ( seconds >= 60) {
      seconds = 0;
      minutes++;
      if ( minutes >= 60) {
        minutes = 0;
        hours++;

        if ( hours >= 24) {
          hours = 0;

        }
      }
    }
    writeDisplay(SECOND, seconds);
    writeDisplay(MINUTE, minutes);
    writeDisplay(HOUR, hours);
  }

  /*
     NTP Getter region, get the time from a ntp server specified in "ntpServer" with a periodicity of "ntpGetPeriod" miliseconds.
  */
  tmpCalc = millis() - ntpMillisValue;
  if (tmpCalc > ntpGetPeriod || isConnecting) {
    ntpMillisValue = millis();
    if (tmpCalc - ntpGetPeriod > 60000) { // If it fails after 60s, retry again in the next period
      finishNtpGetter();
      return;
    }
    if (!isConnecting) {
      WiFi.begin(ssid, password);
      isConnecting = true;
    }
    if (WiFi.status() == WL_CONNECTED) {
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      if (getLocalTime(&timeinfo)) {
        hours = timeinfo.tm_hour;
        minutes = timeinfo.tm_min;
        seconds = timeinfo.tm_sec;
        finishNtpGetter();
      }
    }

  }
}

void finishNtpGetter() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  isConnecting = false;
  ntpMillisValue = millis();
}

void shuffle() {
  if (!isShufflingMinutes) {
    isShufflingHours = false;
  }
  if (!isShufflingSeconds) {
    isShufflingMinutes = false;
  }
  if (seconds == 30 ) { // Shuffles every minute at the 30s mark.
    isShufflingSeconds = true;
    isShufflingMinutes = true;
    isShufflingHours = true;
  }
  else {
    isShufflingSeconds = false;
  }
}

/*
   ex: 26 = 0001 1010‬
   decadeSeparator(26) = 0110  0010
                         (6)   (2)
*/
byte decadeSeparator(byte num) {
  byte separated = num - (num / 10) * 10;
  separated--;
  if (separated < 0 || separated > 9)
    separated = 9;
  separated = separated << 4;
  byte dec = num / 10 - 1;
  if (dec < 0 || dec > 9)
    dec = 9;
  separated = separated + dec;
  return separated;
}


/*
    h - hours
    m - minutes
    s - seconds
*/
void writeDisplay(DigitType type, byte value) {
  value = decadeSeparator(value);
  byte bitSelector = 0b10000000;
  bool val = false;
  digitalWrite(STR_H, type == HOUR);
  digitalWrite(STR_M, type == MINUTE);
  digitalWrite(STR_S, type == SECOND);
  for (int i = 0; i < 8; i++) {
    val = value & bitSelector;
    digitalWrite(DATA_PIN, val);
    digitalWrite(CLK_PIN, LOW);
    delay(CLK);
    digitalWrite(CLK_PIN, HIGH);
    delay(CLK);
    digitalWrite(CLK_PIN, LOW);
    bitSelector = bitSelector >> 1;
  }
}
