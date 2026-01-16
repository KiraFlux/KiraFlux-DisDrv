#pragma once

#include <Wire.h>
#include <kf/aliases.hpp>

#include "kf/DisplayDriver.hpp"

namespace kf {

/// @brief SSD1306 OLED display (128x64)
struct SSD1306 : DisplayDriver<SSD1306, u8, 128, 64, /* monochrome = */ true> {
    friend Self;

private:
    /// @brief I2C device address
    const u8 address;

public:
    /// @brief Constructor with address configuration
    explicit SSD1306(u8 address = 0x3C) :
        address{address} {}

    /// @brief Set contrast (0-255)
    void setContrast(u8 value) const {
        Wire.beginTransmission(address);
        (void) Wire.write(CommandMode);
        (void) Wire.write(Contrast);
        (void) Wire.write(value);
        (void) Wire.endTransmission();
    }

    /// @brief Enable/disable display power
    void setPower(bool on) {
        sendCommand(on ? DisplayOn : DisplayOff);
    }

    /// @brief Color inversion
    void invert(bool invert) {
        sendCommand(invert ? InvertDisplay : NormalDisplay);
    }

private:
    // DisplayDriver implementation

    kf_nodiscard static u8 getWidthImpl() { return phys_width; }

    kf_nodiscard static u8 getHeightImpl() { return phys_height; }

    kf_nodiscard bool initImpl() const {
        static constexpr u8 init_commands[] = {
            CommandMode,

            // Turn off for safe configuration
            DisplayOff,

            // Clock divider
            ClockDiv, 0x80,

            // Enable internal charge pump
            ChargePump, 0x14,

            // Horizontal addressing mode
            AddressingMode, Horizontal,

            // Default contrast 127
            Contrast, 0x7F,

            // VCOM voltage
            SetVcomDetect, 0x40,

            // Normal orientation
            NormalH, NormalV,

            // Turn display on
            DisplayOn,

            // Pin configuration (128x64)
            SetComPins, 0x12,

            // Multiplex (64 lines)
            SetMultiplex, 0x3F};

        if (not Wire.begin()) { return false; }

        Wire.beginTransmission(address);

        const auto written = Wire.write(init_commands, sizeof(init_commands));
        if (sizeof(init_commands) != written) { return false; }

        const u8 end_transmission_code = Wire.endTransmission();
        return 0 == end_transmission_code;
    }

    void sendImpl() {
        static constexpr auto packet_size = 64;// Optimal for ESP32 performance

        static constexpr u8 set_area_commands[] = {
            CommandMode,
            // Set full display window
            ColumnAddr,
            0,
            max_phys_x,
            PageAddr,
            0,
            max_page,
        };

        Wire.beginTransmission(address);
        (void) Wire.write(set_area_commands, sizeof(set_area_commands));
        (void) Wire.endTransmission();

        auto p = software_screen_buffer;
        const auto *end = p + sizeof(software_screen_buffer);

        while (p < end) {
            Wire.beginTransmission(address);
            (void) Wire.write(Command::DataMode);
            (void) Wire.write(p, packet_size);
            (void) Wire.endTransmission();

            p += packet_size;
        }
    }

    // flip* only
    void setOrientationImpl(Orientation orientation) {
        constexpr auto flip_x = 0b01;
        constexpr auto flip_y = 0b10;

        const u8 flags = static_cast<int>(orientation) & (flip_x | flip_y);
        sendCommand((flags & flip_x) ? FlipH : NormalH);
        sendCommand((flags & flip_y) ? FlipV : NormalV);
    }

    // SSD1306 command protocol

    enum Command : u8 {
        DisplayOff = 0xAE,
        DisplayOn = 0xAF,

        CommandMode = 0x00,
        OneCommandMode = 0x80,
        DataMode = 0x40,

        AddressingMode = 0x20,
        Horizontal = 0x00,
        Vertical = 0x01,

        NormalV = 0xC8,
        FlipV = 0xC0,
        NormalH = 0xA1,
        FlipH = 0xA0,

        Contrast = 0x81,
        SetComPins = 0xDA,
        SetVcomDetect = 0xDB,
        ClockDiv = 0xD5,
        SetMultiplex = 0xA8,
        ColumnAddr = 0x21,
        PageAddr = 0x22,
        ChargePump = 0x8D,

        NormalDisplay = 0xA6,
        InvertDisplay = 0xA7
    };

    /// @brief Send single command
    void sendCommand(Command command) const {
        Wire.beginTransmission(address);
        (void) Wire.write(OneCommandMode);
        (void) Wire.write(static_cast<u8>(command));
        (void) Wire.endTransmission();
    }
};

}// namespace kf