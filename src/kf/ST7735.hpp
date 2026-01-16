#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <kf/aliases.hpp>
#include <kf/pixel_traits.hpp>

#include "kf/DisplayDriver.hpp"

namespace kf {

struct ST7735 : DisplayDriver<ST7735, PixelFormat::RGB565, 128, 160> {
    friend Base;

private:
    enum MadCtl : u8 {
        RgbMode = 0x00,
        BgrMode = 0x08,

        MirrorTranspose = 0x20,
        MirrorX = 0x40,
        MirrorY = 0x80,
    };

public:
    struct Settings {
        Orientation orientation;
        u8 pin_spi_slave_select;
        u8 pin_data_command;
        u8 pin_reset;
        u32 spi_frequency;

        constexpr explicit Settings(
            gpio_num_t spi_cs,
            gpio_num_t dc,
            gpio_num_t reset,
            u32 spi_freq = 27000000u,
            Orientation orientation = Orientation::Normal) :
            pin_spi_slave_select{static_cast<u8>(spi_cs)},
            pin_data_command{static_cast<u8>(dc)},
            pin_reset{static_cast<u8>(reset)},
            spi_frequency{spi_freq},
            orientation{orientation} {}
    };

private:
    const Settings &settings;
    SPIClass &spi;

    u8 logical_width{phys_width};
    u8 logical_height{phys_height};
    u8 madctl_base_mode{MadCtl::BgrMode};

public:
    explicit ST7735(const Settings &settings, SPIClass &spi_instance) :
        settings{settings}, spi{spi_instance} {}

private:
    // Display driver Impl

    kf_nodiscard u8 getWidthImpl() const { return logical_width; }

    kf_nodiscard u8 getHeightImpl() const { return logical_height; }

    kf_nodiscard bool initImpl() {
        pinMode(settings.pin_spi_slave_select, OUTPUT);
        pinMode(settings.pin_data_command, OUTPUT);
        pinMode(settings.pin_reset, OUTPUT);

        spi.begin();
        spi.setFrequency(settings.spi_frequency);

        digitalWrite(settings.pin_reset, LOW);
        delay(10);
        digitalWrite(settings.pin_reset, HIGH);
        delay(120);

        sendCommand(Command::SWRESET);
        delay(150);

        sendCommand(Command::SLPOUT);
        delay(255);

        sendCommand(Command::COLMOD);
        const u8 color_mode{0x05};// 16-bit color
        sendData(&color_mode, sizeof(color_mode));

        setOrientation(settings.orientation);

        sendCommand(Command::DISPON);
        delay(100);

        return true;
    }

    void sendImpl() const {
        sendCommand(Command::RAMWR);
        sendData(reinterpret_cast<const u8 *>(software_screen_buffer), sizeof(software_screen_buffer));
    }

    void setOrientationImpl(Orientation orientation) {
        constexpr u8 orient_to_transform[]{
            // Orientation::Normal
            0,

            // Orientation::MirrorX
            MadCtl::MirrorX,

            // Orientation::MirrorY
            MadCtl::MirrorY,

            // Orientation::Flip
            MadCtl::MirrorX | MadCtl::MirrorY,

            // Orientation::ClockWise
            MadCtl::MirrorX | MadCtl::MirrorTranspose,

            // Orientation::CounterClockWise
            MadCtl::MirrorY | MadCtl::MirrorTranspose,
        };

        const u8 madctl = madctl_base_mode | orient_to_transform[static_cast<int>(orientation)];

        if (madctl & MadCtl::MirrorTranspose) {
            logical_width = phys_height;
            logical_height = phys_width;
        } else {
            logical_width = phys_width;
            logical_height = phys_height;
        }

        sendCommand(Command::MADCTL);
        sendData(&madctl, sizeof(madctl));

        u8 data[4] = {0x00, 0x00, 0x00, static_cast<u8>(logical_width - 1)};
        sendCommand(Command::CASET);
        sendData(data, sizeof(data));

        data[3] = logical_height - 1;
        sendCommand(Command::RASET);
        sendData(data, sizeof(data));
    }

    // Display driver protocol

    void sendData(const u8 *data, usize size) const {
        digitalWrite(settings.pin_data_command, HIGH);
        digitalWrite(settings.pin_spi_slave_select, LOW);
        spi.writeBytes(data, size);
        digitalWrite(settings.pin_spi_slave_select, HIGH);
    }

    enum class Command : u8 {
        SWRESET = 0x01,

        SLPIN = 0x10,
        SLPOUT = 0x11,

        INVOFF = 0x20,
        INVON = 0x21,
        DISPOFF = 0x28,
        DISPON = 0x29,
        CASET = 0x2A,
        RASET = 0x2B,
        RAMWR = 0x2C,

        MADCTL = 0x36,
        COLMOD = 0x3A
    };

    void sendCommand(Command command) const {
        digitalWrite(settings.pin_data_command, LOW);
        digitalWrite(settings.pin_spi_slave_select, LOW);
        spi.write(static_cast<u8>(command));
        digitalWrite(settings.pin_spi_slave_select, HIGH);
    }
};

}// namespace kf