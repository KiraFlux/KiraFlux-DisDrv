#pragma once

#include <kf/attributes.hpp>
#include <kf/pixel_traits.hpp>
#include <kf/slice.hpp>

namespace kf {

/// @brief Display Driver static Interface
/// @tparam Impl Derived type
/// @tparam Format pixel format
/// @tparam W Display width
/// @tparam H Display height
template<typename Impl, PixelFormat Format, u8 W, u8 H> struct DisplayDriver {
    friend Impl;

protected:
    using Base = DisplayDriver;
    using Traits = PixelTraits<Format>;

public:
    using BufferType = typename Traits::BufferType;
    using ColorType = typename Traits::ColorType;

protected:
    static constexpr auto phys_width{W};
    static constexpr auto max_phys_x{phys_width - 1};

    static constexpr auto phys_height{H};
    static constexpr auto max_phys_y{phys_height - 1};

    static constexpr auto buffer_items{Traits::template buffer_size<W, H>};

    BufferType software_screen_buffer[buffer_items]{};

public:
    /// @brief Display Orientation
    enum class Orientation : u8 {
        /// @brief normal
        Normal = 0,

        /// @brief Mirror Only X axis
        MirrorX = 1,// 0b01

        /// @brief Mirror Only Y axis
        MirrorY = 2,// 0b10

        /// @brief Mirror X and Y axis or rotate 180 deg
        Flip = 3,// 0b11

        /// @brief Rotate 90 deg
        ClockWise = 4,

        /// @brief Rotate -90 deg
        CounterClockWise = 5,
    };

    /// @brief Initialize display driver
    // kf_nodiscard bool initImpl() {}
    kf_nodiscard bool init() { return impl().initImpl(); }

    /// @brief Get display width in pixels
    // kf_nodiscard u8 getWidthImpl() const {}
    kf_nodiscard u8 width() const { return c_impl().getWidthImpl(); }

    /// @brief Get display height in pixels
    // kf_nodiscard u8 getHeightImpl() const {}
    kf_nodiscard u8 height() const { return c_impl().getHeightImpl(); }

    /// @brief Send buffer on display
    // void sendImpl() {}
    void send() const { c_impl().sendImpl(); }

    /// @brief Set display Orientation
    // void setOrientationImpl(Orientation orientation) const {}
    void setOrientation(Orientation orientation) { impl().setOrientationImpl(orientation); }

    //

    /// @brief Get Software display buffer
    kf_nodiscard slice<BufferType> buffer() { return {software_screen_buffer, buffer_items}; }

    /// @brief Get Max X position
    kf_nodiscard u8 maxX() const { return width() - 1; }

    /// @brief Get Max Y position
    kf_nodiscard u8 maxY() const { return height() - 1; }

private:
    inline Impl &impl() { return *static_cast<Impl *>(this); }

    inline const Impl &c_impl() const { return *static_cast<const Impl *>(this); }
};

}// namespace kf