# KiraFlux-DisDrv

Библиотека драйверов дисплеев для микроконтроллеров с единым интерфейсом.

## Драйверы

* **SSD1306**: монохромный OLED, 128×64 пикселей, I2C
* **ST7735**: цветной TFT, 128×160 пикселей, 16-битный цвет (RGB565), SPI

## Особенности

* **Единый API**: базовый класс `DisplayDriver` с методами `init()`, `send()`, `width()`, `height()`, `setOrientation()`
* **CRTP**: статический полиморфизм через шаблон `DisplayDriver<Impl, ...>`
* **Доступ к буферу**: метод `buffer()` возвращает `kf::slice<BufferItem>` для работы с данными
* **Ориентация**: поддержка отражений и поворотов через `setOrientation()`

## Быстрый старт

### Установка

```ini
lib_deps =
    https://github.com/KiraFlux/KiraFlux-DisDrv.git
    https://github.com/KiraFlux/KiraFlux-ToolBox.git
```

### SSD1306

```cpp
#include <kf/SSD1306.hpp>
#include <Wire.h>

kf::SSD1306 oled{0x3C};

void setup() {
    Wire.begin();
    oled.init();
    
    auto buf = oled.buffer();
    for (usize i = 0; i < buf.size(); ++i) buf.data()[i] = 0xAA;
    
    oled.send();
}
```

### ST7735

```cpp
#include <kf/ST7735.hpp>
#include <SPI.h>

SPIClass hspi{HSPI};
kf::ST7735::Settings tft_settings{15, 2, 4};
kf::ST7735 tft{tft_settings, hspi};

void setup() {
    tft.init();
    
    auto buf = tft.buffer();
    for (usize i = 0; i < buf.size(); ++i) buf.data()[i] = 0xF800;
    
    tft.send();
}
```

## API

### DisplayDriver

```cpp
bool init();
u8 width() const;
u8 height() const;
void send() const;
void setOrientation(Orientation orientation);
kf::slice<BufferItem> buffer();
u8 maxX() const;
u8 maxY() const;

enum class Orientation : u8 {
    Normal = 0,
    MirrorX = 1,
    MirrorY = 2,
    Flip = 3,
    ClockWise = 4,
    CounterClockWise = 5
};
```

### SSD1306

```cpp
explicit SSD1306(u8 address = 0x3C);
void setContrast(u8 value) const;
void setPower(bool on);
void invert(bool invert);
```

### ST7735

```cpp
struct Settings {
    u8 pin_spi_slave_select;
    u8 pin_data_command;
    u8 pin_reset;
    u32 spi_frequency;
    Orientation orientation;
};
explicit ST7735(const Settings &settings, SPIClass &spi_instance);
```

## Формат буфера

* **SSD1306**: `slice<u8>` размером 1024 байта. Биты в каждом байте — вертикальные пиксели.
* **ST7735**: `slice<u16>` размером 128×160 элементов. RGB565 формат.

## Лицензия

MIT License