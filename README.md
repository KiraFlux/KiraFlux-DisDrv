# KiraFlux OLED

**Минималистичная библиотека для работы с OLED дисплеями SSD1306 128x64 по I2C**

## Особенности

- **Прямой доступ к буферу**: 1024-байтный буфер для управления пикселями
- **Оптимизированное обновление**: Пакетная передача данных по 64 байта
- **Полный контроль**: Настройка контраста, ориентации, инверсии
- **Высокая производительность**: До 74 FPS на ESP32

## Установка

PlatformIO (`platformio.ini`):
```ini
lib_deps = https://github.com/KiraFlux/KiraFlux-OLED.git
```

## Быстрый старт

```cpp
#include <kf/SSD1306.h>

kf::SSD1306 oled{};

void setup() {
    Wire.begin();
    oled.init();      // Инициализация дисплея
    oled.flush();     // Первый вывод
}

void loop() {
    // Очистка буфера
    memset(oled.buffer, 0, sizeof(oled.buffer));
    
    // Рисование в буфер
    for (int x = 0; x < oled.width(); x++) {
        oled.buffer[x] = 0xFF;  // Верхняя граница
    }
    
    oled.flush();  // Обновление дисплея
    delay(1000);
}
```

## API Reference

### Конструктор
```cpp
kf::SSD1306 oled{};           // Адрес 0x3C
kf::SSD1306 oled{0x3D};       // Пользовательский адрес
```

### Основные методы
- `bool init()` - Инициализация дисплея
- `void flush()` - Обновление дисплея из буфера
- `void setContrast(u8 value)` - Установка яркости (0-255)
- `void setPower(bool on)` - Включение/выключение питания
- `void flipHorizontal(bool flip)` - Отражение по горизонтали
- `void flipVertical(bool flip)` - Отражение по вертикали
- `void invert(bool invert)` - Инверсия цветов

### Свойства дисплея
- `u8 width()` - Ширина дисплея (128)
- `u8 height()` - Высота дисплея (64)
- `u8 maxX()` - Максимальная координата X (127)
- `u8 maxPage()` - Максимальный индекс страницы (7)

### Буфер дисплея
```cpp
u8 buffer[1024];  // 128 × 8 страниц
// Организация:
// Страница 0: y=0-7    → buffer[0]..buffer[127]
// Страница 1: y=8-15   → buffer[128]..buffer[255]
// ...
// Страница 7: y=56-63  → buffer[896]..buffer[1023]
```

## Работа с буфером

### Установка пикселя
```cpp
void setPixel(int x, int y, bool on) {
    if (x < 0 || x >= oled.width() || y < 0 || y >= oled.height()) return;
    
    int page = y / 8;
    int bit = y % 8;
    int index = page * oled.width() + x;
    
    if (on) {
        oled.buffer[index] |= (1 << bit);
    } else {
        oled.buffer[index] &= ~(1 << bit);
    }
}
```

### Очистка экрана
```cpp
memset(oled.buffer, 0, sizeof(oled.buffer));
```

### Рисование горизонтальной линии
```cpp
void drawHLine(int y) {
    for (int x = 0; x < oled.width(); x++) {
        setPixel(x, y, true);
    }
}
```

*Для продвинутой графики рекомендуется использовать [KiraFlux-GFX](https://github.com/KiraFlux/KiraFlux-GFX.git)*

## Лицензия

MIT License