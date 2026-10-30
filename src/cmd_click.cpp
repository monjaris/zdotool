#include "zdotool.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <print>
#include <string_view>
#include <vector>

namespace {

constexpr std::array specs{
    zdotool::OptSpec{"-r", "--repeat", true},
    zdotool::OptSpec{"-D", "--next-delay", true},
    zdotool::OptSpec{"-P", "--press-release", false},
    zdotool::OptSpec{"-h", "--help", false},
};

std::optional<int> parse_button(std::string_view text) {
    if (text.starts_with("0x") || text.starts_with("0X")) {
        text.remove_prefix(2);
    }
    if (text.empty()) {
        return std::nullopt;
    }
    return zdotool::parse_int(text, 16);
}

bool click_once(zdotool::Client& client, int button, int delay_ms, bool press_release) {
    const auto keycode = static_cast<std::uint16_t>((button & 0xf) | BTN_LEFT);

    if (press_release) {
        if (!client.emit(EV_KEY, keycode, 1, true)) {
            return false;
        }
        if (!client.emit(EV_KEY, keycode, 0, true)) {
            return false;
        }
        zdotool::sleep_ms(delay_ms);
        return true;
    }

    if ((button & 0x40) != 0) {
        if (!client.emit(EV_KEY, keycode, 1, true)) {
            return false;
        }
        zdotool::sleep_ms(delay_ms);
    }
    if ((button & 0x80) != 0) {
        if (!client.emit(EV_KEY, keycode, 0, true)) {
            return false;
        }
        zdotool::sleep_ms(delay_ms);
    }
    if ((button & 0xc0) == 0) {
        zdotool::sleep_ms(delay_ms);
    }
    return true;
}

} // namespace

int zdotool::cmd_click(Client& client, int argc, char** argv) {
    Args args{argc, argv, specs};

    if (!args.parse()) {
        std::println(stderr, "click: {}", args.error());
        return 1;
    }

    if (args.has("-h", "--help") || args.positional().empty()) {
        std::print("{}", help::click);
        return 0;
    }

    const int repeats = args.get_int("-r", "--repeat", default_click_repeat);
    const int delay_ms = args.get_int("-D", "--next-delay", default_click_delay_ms);
    if (!args.ok()) {
        std::println(stderr, "click: {}", args.error());
        return 1;
    }
    if (repeats < 0 || delay_ms < 0) {
        std::println(stderr, "click: repeat and delay must be non-negative");
        return 1;
    }

    std::vector<int> buttons;
    buttons.reserve(args.positional().size());
    for (const auto& text : args.positional()) {
        const auto button = parse_button(text);
        if (!button) {
            std::println(stderr, "click: invalid button '{}'", text);
            return 1;
        }
        buttons.push_back(*button);
    }

    const bool press_release = args.has("-P", "--press-release");
    for (int i = 0; i < repeats; ++i) {
        for (const int button : buttons) {
            if (!click_once(client, button, delay_ms, press_release)) {
                return 1;
            }
        }
    }

    return 0;
}
