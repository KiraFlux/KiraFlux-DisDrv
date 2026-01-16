#pragma once

#include <kf/attributes.hpp>
#include <kf/slice.hpp>

namespace kf {

/// @brief Display Driver static Interface
/// @tparam Impl Derived type
/// @tparam T Buffer item type
/// @tparam W Display width
/// @tparam H Display height
/// @tparam IsMonochrome true - display is Monochrome, false - colorful
template<typename Impl, typename T, u8 W, u8 H, bool IsMonochrome> struct DisplayDriver {
    friend Impl;

protected:
    using Self = DisplayDriver;

    using BufferItem = T;

    static constexpr auto item_bits = 8 * sizeof(BufferItem);

    static constexpr auto phys_width{W};
    static constexpr auto max_phys_x{phys_width - 1};

    static constexpr auto phys_height{H};
    static constexpr auto max_phys_y{phys_height - 1};

    static constexpr auto pages{IsMonochrome ? ((phys_height + item_bits - 1) / item_bits) : phys_height};
    static constexpr auto max_page{pages - 1};

    static constexpr auto buffer_items{phys_width * pages};

    BufferItem software_screen_buffer[buffer_items]{};

public:
    /// @brief Initialize display driver
    // kf_nodiscard bool initImpl() {}
    kf_nodiscard bool init() { return impl().initImpl(); }

    /// @brief Get display width in pixels
    // kf_nodiscard u8 getWidthImpl() {}
    kf_nodiscard u8 width() const { return c_impl().getWidthImpl(); }

    /// @brief Get display height in pixels
    // kf_nodiscard u8 getHeightImpl() {}
    kf_nodiscard u8 height() const { return c_impl().getHeightImpl(); }

    /// @brief Send buffer on display
    // void sendImpl() {}
    void send() const { c_impl().sendImpl(); }

    //

    /// @brief Get Software display buffer
    kf_nodiscard slice<BufferItem> buffer() { return {software_screen_buffer, buffer_items}; }

    /// @brief Get Max X position
    kf_nodiscard u8 maxX() const { return width() - 1; }

    /// @brief Get Max Y position
    kf_nodiscard u8 maxY() const { return height() - 1; }

private:
    inline Impl &impl() { return *static_cast<Impl *>(this); }

    inline const Impl &c_impl() const { return *static_cast<const Impl *>(this); }
};

}// namespace kf