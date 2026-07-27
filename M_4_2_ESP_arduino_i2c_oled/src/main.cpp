#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#define OLED_SDA_PIN 8
#define OLED_SCL_PIN 9

// Constructor for 1.3" SH1106 I2C OLED
// Use U8G2_SH1106_128X64_NONAME_F_HW_I2C for 1.3" displays
// Створення об'єкта OLED-дисплея
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// Сканування пристроїв на шині I²C
void scanI2CDevices()
{

  uint8_t found = 0; // Лічильник знайдених пристроїв
  Serial.println("I2C scan start...");

  // Перебір усіх адрес I²C
  for (uint8_t addr = 1; addr < 127; addr++)
  {
    // Відправка запиту за адресою
    Wire.beginTransmission(addr);
    // Перевірка відповіді
    if (Wire.endTransmission() == 0)
    {
      // Виведення знайденої адреси
      Serial.print("Found I2C device at 0x");
      if (addr < 16)
      {
        Serial.print('0');
      }
      Serial.println(addr, HEX);
      found++;
    }
  }

  if (found == 0)
  {
    Serial.println("No I2C devices found");
  }
}

void setup(void)
{
  Serial.begin(115200);
  delay(300);

  // Ініціалізує апаратний інтерфейс I²C
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  scanI2CDevices(); // Сканування шини I²C
  u8g2.begin();     // Ініціалізація OLED-дисплея
}

void loop(void)
{
  // Виведення тексту на OLED-дисплей через бібліотеку U8g2
  u8g2.clearBuffer();                   // Очищення буфера дисплея
  u8g2.setFont(u8g2_font_ncenB08_tr);   // Встановлює шрифт 8 пікселів
  u8g2.drawStr(0, 10, "Hello ESP32!");  // Виведення тексту x=0, y=10
  u8g2.drawStr(0, 50, "1.3 inch OLED"); // Виведення тексту x=0, y=50
  u8g2.sendBuffer();                    // Передача даних на дисплей
  delay(1000);
}

//-----------------------------------------------------------

// #include <Arduino.h>
// #include <U8g2lib.h>
// #include <Wire.h>

// // Определение пинов I2C для вашей платы ESP32-S3 (измените, если используете другие пины)
// #define SDA_PIN 8
// #define SCL_PIN 9

// // Инициализация дисплея SH1106 128x64 I2C (без пина сброса)
// U8X8_HAVE_HW_I2C
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// void drawSplash();

// // Иконка по центру (Красивый футуристичный ромбовидный значок)
// static const unsigned char logo_bits[] PROGMEM = {
//     0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0xC0, 0x00,
//     0x00, 0x07, 0xE0, 0x00, 0x00, 0x0D, 0xB0, 0x00, 0x00, 0x19, 0x98, 0x00,
//     0x00, 0x31, 0x8C, 0x00, 0x00, 0x63, 0xC6, 0x00, 0x00, 0xC7, 0xE3, 0x00,
//     0x01, 0x8D, 0xB1, 0x80, 0x03, 0x19, 0x98, 0xC0, 0x06, 0x31, 0x8C, 0x60,
//     0x0C, 0x63, 0xC6, 0x30, 0x18, 0xC7, 0xE3, 0x18, 0x31, 0x8D, 0xB1, 0x8C,
//     0x31, 0x8D, 0xB1, 0x8C, 0x18, 0xC7, 0xE3, 0x18, 0x0C, 0x63, 0xC6, 0x30,
//     0x06, 0x31, 0x8C, 0x60, 0x03, 0x19, 0x98, 0xC0, 0x01, 0x8D, 0xB1, 0x80,
//     0x00, 0xC7, 0xE3, 0x00, 0x00, 0x63, 0xC6, 0x00, 0x00, 0x31, 0x8C, 0x00,
//     0x00, 0x19, 0x98, 0x00, 0x00, 0x0D, 0xB0, 0x00, 0x00, 0x07, 0xE0, 0x00,
//     0x00, 0x03, 0xC0, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};

// void setup()
// {
//   // Настройка кастомных пинов I2C для ESP32-S3
//   Wire.begin(SDA_PIN, SCL_PIN);

//   // Старт дисплея
//   u8g2.begin();

//   // Вызов анимации заставки
//   drawSplash();
// }

// void drawSplash()
// {
//   // 1. Плавное появление рамки и иконки
//   for (int r = 0; r <= 32; r += 4)
//   {
//     u8g2.clearBuffer();
//     u8g2.drawFrame(0, 0, 128, 64);            // Красивая рамка по контуру
//     u8g2.drawXBMP(48, 16, 32, 32, logo_bits); // Отрисовка нашей картинки
//     u8g2.sendBuffer();
//     delay(30);
//   }

//   delay(500);

//   // 2. Эффект разлетающихся линий под текст
//   for (int x = 64; x >= 10; x -= 6)
//   {
//     u8g2.clearBuffer();
//     u8g2.drawFrame(0, 0, 128, 64);
//     u8g2.drawXBMP(48, 16, 32, 32, logo_bits);
//     u8g2.drawHLine(x, 52, 128 - (x * 2)); // Линия под будущим текстом
//     u8g2.sendBuffer();
//   }

//   // 3. Вывод красивого текста
//   u8g2.setFont(u8g2_font_profont12_mf); // Качественный моноширинный шрифт
//   u8g2.setCursor(24, 60);
//   u8g2.print("ESP32-S3 READY");
//   u8g2.sendBuffer();

//   delay(3000); // Держим заставку 3 секунды
// }

// void loop()
// {
//   // Здесь начинается основной код вашего интерфейса
//   u8g2.clearBuffer();
//   u8g2.setFont(u8g2_font_ncenB08_tr);
//   u8g2.drawStr(10, 25, "System Active");
//   u8g2.drawBox(10, 35, 108, 8); // Имитация прогресс-бара или рабочего стола
//   u8g2.sendBuffer();
//   delay(1000);
// }

//-----------------------------------------------------------

// #include <Arduino.h>
// #include <U8g2lib.h>
// #include <Wire.h>

// // Конфигурация пинов I2C для ESP32-S3
// #define OLED_SDA 8
// #define OLED_SCL 9

// // Инициализация Full-Buffer для SH1106 128x64 I2C
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// // Прототипы функций (чтобы компилятор в PlatformIO не выдавал ошибку)
// void drawIntroAnimation();
// void showSystemStatus();

// // Массив байт для красивой иконки "Процессор/Чип" по центру (32x32 пикселя)
// static const unsigned char epd_bitmap_cpu[] PROGMEM = {
//     0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x7e, 0x00, 0x00, 0xdb, 0xdb, 0x00,
//     0x00, 0xdb, 0xdb, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0xfe, 0x3c, 0x3c, 0x7f,
//     0x1b, 0xff, 0xff, 0xd8, 0x1b, 0xc3, 0xc3, 0xd8, 0x1b, 0xc3, 0xc3, 0xd8,
//     0x1b, 0xc3, 0xc3, 0xd8, 0x1b, 0xc3, 0xc3, 0xd8, 0x1b, 0xff, 0xff, 0xd8,
//     0x1b, 0xff, 0xff, 0xd8, 0x1b, 0xc3, 0xc3, 0xd8, 0x1b, 0xc3, 0xc3, 0xd8,
//     0x1b, 0xc3, 0xc3, 0xd8, 0x1b, 0xc3, 0xc3, 0xd8, 0x1b, 0xff, 0xff, 0xd8,
//     0xfe, 0x3c, 0x3c, 0x7f, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xdb, 0xdb, 0x00,
//     0x00, 0xdb, 0xdb, 0x00, 0x00, 0x7e, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00};

// void setup()
// {
//   // Запуск шины I2C на кастомных пинах ESP32-S3
//   Wire.begin(OLED_SDA, OLED_SCL);

//   // Инициализация дисплея
//   u8g2.begin();

//   // Запуск красивой анимации заставки
//   drawIntroAnimation();
// }

// void loop()
// {
//   // Основной цикл: вывод рабочего экрана системы после заставки
//   showSystemStatus();
//   delay(100);
// }

// // Функция анимации заставки (Эффект загрузки и сканирования кибер-интерфейса)
// void drawIntroAnimation()
// {
//   // Шаг 1: Эффект "схлопывания" горизонтальной линии в рамку экрана
//   for (int w = 0; w <= 64; w += 4)
//   {
//     u8g2.clearBuffer();
//     u8g2.drawHLine(64 - w, 32, w * 2);
//     u8g2.sendBuffer();
//     delay(15);
//   }

//   // Шаг 2: Расширение линии до размеров полноценной рамки
//   for (int h = 0; h <= 32; h += 2)
//   {
//     u8g2.clearBuffer();
//     u8g2.drawFrame(0, 32 - h, 128, h * 2);
//     u8g2.sendBuffer();
//     delay(10);
//   }

//   // Шаг 3: Процедурный эффект сканирования сетки и проявление логотипы
//   for (int scanY = 2; scanY < 62; scanY += 3)
//   {
//     u8g2.clearBuffer();
//     u8g2.drawFrame(0, 0, 128, 64); // Внешняя рамка

//     // Рисуем логотип по центру с постепенным "проявлением"
//     u8g2.drawXBMP(48, 16, 32, 32, epd_bitmap_cpu);

//     // Линия сканирования радара
//     u8g2.drawHLine(2, scanY, 124);
//     u8g2.drawHLine(2, scanY + 1, 124);

//     u8g2.sendBuffer();
//     delay(20);
//   }

//   // Шаг 4: Появление текста приветствия с эффектом мерцания (Blink)
//   for (int i = 0; i < 3; i++)
//   {
//     u8g2.clearBuffer();
//     u8g2.drawFrame(0, 0, 128, 64);
//     u8g2.drawXBMP(48, 12, 32, 32, epd_bitmap_cpu);
//     u8g2.setFont(u8g2_font_6x10_tf);
//     u8g2.drawStr(22, 56, "ESP32-S3 BOOTING");
//     u8g2.sendBuffer();
//     delay(250);

//     u8g2.clearBuffer();
//     u8g2.drawFrame(0, 0, 128, 64);
//     u8g2.drawXBMP(48, 12, 32, 32, epd_bitmap_cpu);
//     u8g2.sendBuffer();
//     delay(150);
//   }

//   // Финальный аккорд заставки перед переходом в основное меню
//   u8g2.clearBuffer();
//   u8g2.drawFrame(0, 0, 128, 64);
//   u8g2.drawXBMP(48, 12, 32, 32, epd_bitmap_cpu);
//   u8g2.setFont(u8g2_font_6x10_tf);
//   u8g2.drawStr(34, 56, "SYSTEM READY");
//   u8g2.sendBuffer();
//   delay(1500); // Удерживаем финальную картинку 1.5 секунды
// }

// // Пример отрисовки основного экрана после заставки
// void showSystemStatus()
// {
//   static uint32_t progress = 0;
//   progress = (progress + 1) % 100;

//   u8g2.clearBuffer();

//   // Верхний бар статуса
//   u8g2.setFont(u8g2_font_profont10_mf);
//   u8g2.drawStr(4, 10, "CORE: OK");
//   u8g2.drawStr(80, 10, "WiFi: OFF");
//   u8g2.drawHLine(0, 13, 128);

//   // Вывод данных по центру
//   // u8g2.setFont(u8g2_font_fixed_v0_tr);
//   u8g2.setFont(u8g2_font_profont12_mf);
//   u8g2.setCursor(4, 32);
//   u8g2.print("Uptime: ");
//   u8g2.print(millis() / 1000);
//   u8g2.print("s");

//   // Динамический прогресс-бар
//   u8g2.drawFrame(4, 44, 120, 10);
//   u8g2.drawBox(6, 46, (progress * 116) / 100, 6);

//   u8g2.sendBuffer();
// }

//-----------------------------------------------------------