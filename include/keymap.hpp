#pragma once

#include "common.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace zdotool {

struct KeyDef {
    std::uint16_t code = 0; // 0 (KEY_RESERVED) means "no mapping"
    bool shift = false;
    bool ctrl = false;

    [[nodiscard]] constexpr bool valid() const noexcept { return code != 0; }
};

inline constexpr KeyDef NONE{};
inline constexpr KeyDef K(std::uint16_t code) { return {code, false, false}; }
inline constexpr KeyDef S(std::uint16_t code) { return {code, true, false}; }
inline constexpr KeyDef C(std::uint16_t code) { return {code, false, true}; }

// US keyboard layout, used by `type`.
inline constexpr std::array<KeyDef, 128> ascii_typing_map = {
    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    NONE, K(KEY_TAB), K(KEY_ENTER), NONE, NONE, NONE, NONE, NONE,
    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
    K(KEY_SPACE), S(KEY_1), S(KEY_APOSTROPHE), S(KEY_3), S(KEY_4), S(KEY_5), S(KEY_7), K(KEY_APOSTROPHE),
    S(KEY_9), S(KEY_0), S(KEY_8), S(KEY_EQUAL), K(KEY_COMMA), K(KEY_MINUS), K(KEY_DOT), K(KEY_SLASH),
    K(KEY_0), K(KEY_1), K(KEY_2), K(KEY_3), K(KEY_4), K(KEY_5), K(KEY_6), K(KEY_7),
    K(KEY_8), K(KEY_9), S(KEY_SEMICOLON), K(KEY_SEMICOLON), S(KEY_COMMA), K(KEY_EQUAL), S(KEY_DOT), S(KEY_SLASH),
    S(KEY_2), S(KEY_A), S(KEY_B), S(KEY_C), S(KEY_D), S(KEY_E), S(KEY_F), S(KEY_G),
    S(KEY_H), S(KEY_I), S(KEY_J), S(KEY_K), S(KEY_L), S(KEY_M), S(KEY_N), S(KEY_O),
    S(KEY_P), S(KEY_Q), S(KEY_R), S(KEY_S), S(KEY_T), S(KEY_U), S(KEY_V), S(KEY_W),
    S(KEY_X), S(KEY_Y), S(KEY_Z), K(KEY_LEFTBRACE), K(KEY_BACKSLASH), K(KEY_RIGHTBRACE), S(KEY_6), S(KEY_MINUS),
    K(KEY_GRAVE), K(KEY_A), K(KEY_B), K(KEY_C), K(KEY_D), K(KEY_E), K(KEY_F), K(KEY_G),
    K(KEY_H), K(KEY_I), K(KEY_J), K(KEY_K), K(KEY_L), K(KEY_M), K(KEY_N), K(KEY_O),
    K(KEY_P), K(KEY_Q), K(KEY_R), K(KEY_S), K(KEY_T), K(KEY_U), K(KEY_V), K(KEY_W),
    K(KEY_X), K(KEY_Y), K(KEY_Z), S(KEY_LEFTBRACE), S(KEY_BACKSLASH), S(KEY_RIGHTBRACE), S(KEY_GRAVE), NONE,
};

// Terminal input, used by `stdin`. Control characters are emitted with Ctrl.
inline constexpr std::array<KeyDef, 128> ascii_stdin_map = {
    NONE, C(KEY_A), C(KEY_B), C(KEY_C), C(KEY_D), C(KEY_E), C(KEY_F), C(KEY_G),
    K(KEY_BACKSPACE), K(KEY_TAB), K(KEY_ENTER), C(KEY_K), C(KEY_L), C(KEY_M), C(KEY_N), C(KEY_O),
    C(KEY_P), C(KEY_Q), C(KEY_R), C(KEY_S), C(KEY_T), C(KEY_U), C(KEY_V), C(KEY_W),
    C(KEY_X), C(KEY_Y), C(KEY_Z), K(KEY_ESC), NONE, NONE, NONE, NONE,
    K(KEY_SPACE), S(KEY_1), S(KEY_APOSTROPHE), S(KEY_3), S(KEY_4), S(KEY_5), S(KEY_7), K(KEY_APOSTROPHE),
    S(KEY_9), S(KEY_0), S(KEY_8), S(KEY_EQUAL), K(KEY_COMMA), K(KEY_MINUS), K(KEY_DOT), K(KEY_SLASH),
    K(KEY_0), K(KEY_1), K(KEY_2), K(KEY_3), K(KEY_4), K(KEY_5), K(KEY_6), K(KEY_7),
    K(KEY_8), K(KEY_9), S(KEY_SEMICOLON), K(KEY_SEMICOLON), S(KEY_COMMA), K(KEY_EQUAL), S(KEY_DOT), S(KEY_SLASH),
    S(KEY_2), S(KEY_A), S(KEY_B), S(KEY_C), S(KEY_D), S(KEY_E), S(KEY_F), S(KEY_G),
    S(KEY_H), S(KEY_I), S(KEY_J), S(KEY_K), S(KEY_L), S(KEY_M), S(KEY_N), S(KEY_O),
    S(KEY_P), S(KEY_Q), S(KEY_R), S(KEY_S), S(KEY_T), S(KEY_U), S(KEY_V), S(KEY_W),
    S(KEY_X), S(KEY_Y), S(KEY_Z), K(KEY_LEFTBRACE), K(KEY_BACKSLASH), K(KEY_RIGHTBRACE), S(KEY_6), S(KEY_MINUS),
    K(KEY_GRAVE), K(KEY_A), K(KEY_B), K(KEY_C), K(KEY_D), K(KEY_E), K(KEY_F), K(KEY_G),
    K(KEY_H), K(KEY_I), K(KEY_J), K(KEY_K), K(KEY_L), K(KEY_M), K(KEY_N), K(KEY_O),
    K(KEY_P), K(KEY_Q), K(KEY_R), K(KEY_S), K(KEY_T), K(KEY_U), K(KEY_V), K(KEY_W),
    K(KEY_X), K(KEY_Y), K(KEY_Z), S(KEY_LEFTBRACE), S(KEY_BACKSLASH), S(KEY_RIGHTBRACE), S(KEY_GRAVE), K(KEY_BACKSPACE),
};

[[nodiscard]] constexpr KeyDef lookup(const std::array<KeyDef, 128>& map,
                                      unsigned char character) noexcept {
    return static_cast<std::size_t>(character) < map.size() ? map[character] : KeyDef{};
}

} // namespace zdotool
