#include <RTClib.h> // For DS3231 or DS1307 RTC module
#include <U8g2lib.h>
#include <Wire.h>

#define I2C_SDA 8
#define I2C_SCL 9

// RTC object (change to RTC_DS3231 for DS3231 module)
RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Створення об'єкта OLED-дисплея
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup()
{
  Serial.begin(115200);

  // Ініціалізація I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialization RTC
  if (!rtc.begin())
  {
    Serial.println("RTC not found");
    while (1)
      delay(10);
  }

  if (!rtc.isrunning())
  {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // July 29, 2026 at 18 you would call:
    // rtc.adjust(DateTime(2026, 7, 29, 18, 0, 0));
  }

  // Ініціалізація OLED
  u8g2.begin();
}

void loop()
{
  // Получаем текущее время из RTC
  DateTime now = rtc.now();

  // Формуємо строки
  String yearStr = String(now.year());
  String monthStr = (now.month() < 10 ? "0" : "") + String(now.month());
  String dayStr = (now.day() < 10 ? "0" : "") + String(now.day());

  String hourStr = (now.hour() < 10 ? "0" : "") + String(now.hour());
  String minuteStr = (now.minute() < 10 ? "0" : "") + String(now.minute());
  String secondStr = (now.second() < 10 ? "0" : "") + String(now.second());

  String dayOfWeek = daysOfTheWeek[now.dayOfTheWeek()];

  String dateStr = yearStr + "-" + monthStr + "-" + dayStr;
  String timeStr = hourStr + ":" + minuteStr + ":" + secondStr;

  // Вывод на OLED через U8g2
  u8g2.clearBuffer(); // Очищення буфера дисплея

  u8g2.setFont(u8g2_font_ncenB08_tr); // // Встановлює шрифт 8 пікселів

  u8g2.drawStr(5, 15, dayOfWeek.c_str()); // Виведення тексту
  u8g2.drawStr(5, 35, dateStr.c_str());   // Виведення тексту
  u8g2.drawStr(5, 55, timeStr.c_str());   // Виведення тексту

  u8g2.sendBuffer(); // Передача даних на дисплей

  delay(1000);
}
