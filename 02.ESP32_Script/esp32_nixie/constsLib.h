#define CLK_PIN 22
#define CLK 0
#define OE_H 39
#define OE_M 34
#define OE_S 35
#define STR_H 32
#define STR_M 33
#define STR_S 25
#define DATA_PIN 26
#define BTN_H 27
#define BTN_M 14
#define TEMP_PIN 12
#define RESET 13
#define OFF_CODE 0b1111

const char *ssid = "";
const char *password = "";
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
int daylightOffset_sec = 0;
const int ntpGetPeriod = 18000000;
enum DigitType
{
  SECOND,
  MINUTE,
  HOUR
};

void writeDisplay(DigitType type, byte value);