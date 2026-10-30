#include "zdotool.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <print>
#include <string_view>
#include <vector>

namespace {

constexpr std::array specs{
    zdotool::OptSpec{"-d", "--key-delay", true},
    zdotool::OptSpec{"-h", "--help", false},
};

struct KeyStroke {
    std::uint16_t code = 0;
    bool pressed = true;
};

std::optional<KeyStroke> parse_key(std::string_view text) {
    bool pressed = true;

    if (const auto colon = text.find(':'); colon != std::string_view::npos) {
        pressed = text.substr(colon + 1) != "0";
        text = text.substr(0, colon);
    }

    int base = 10;
    if (text.starts_with("0x") || text.starts_with("0X")) {
        base = 16;
        text.remove_prefix(2);
    }

    const auto code = zdotool::parse_int(text, base);
    if (!code || *code < 0 || *code > 0xffff) {
        return std::nullopt;
    }

    return KeyStroke{static_cast<std::uint16_t>(*code), pressed};
}

} // namespace

int zdotool::cmd_key(Client& client, int argc, char** argv) {
    Args args{argc, argv, specs};

    if (!args.parse()) {
        std::println(stderr, "key: {}", args.error());
        return 1;
    }

    if (args.has("-h", "--help") || args.positional().empty()) {
        std::print("{}", help::key);
        return 0;
    }

    const int delay_ms = args.get_int("-d", "--key-delay", default_key_command_delay_ms);
    if (!args.ok()) {
        std::println(stderr, "key: {}", args.error());
        return 1;
    }
    if (delay_ms < 0) {
        std::println(stderr, "key: delay must be non-negative");
        return 1;
    }

    std::vector<KeyStroke> strokes;
    strokes.reserve(args.positional().size());
    for (const auto& text : args.positional()) {
        const auto stroke = parse_key(text);
        if (!stroke) {
            std::println(stderr, "key: invalid keycode '{}'", text);
            return 1;
        }
        strokes.push_back(*stroke);
    }

    for (const auto& stroke : strokes) {
        if (!client.emit(EV_KEY, stroke.code, stroke.pressed ? 1 : 0, true)) {
            return 1;
        }
        sleep_ms(delay_ms);
    }

    return 0;
}
