#include "zdotool.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <print>

namespace {

constexpr std::array specs{
    zdotool::OptSpec{"-w", "--wheel", false},
    zdotool::OptSpec{"-a", "--absolute", false},
    zdotool::OptSpec{"-x", "--xpos", true},
    zdotool::OptSpec{"-y", "--ypos", true},
    zdotool::OptSpec{"-h", "--help", false},
};

constexpr auto absolute_reset = std::numeric_limits<std::int32_t>::min();

} // namespace

int zdotool::cmd_mousemove(Client& client, int argc, char** argv) {
    Args args{argc, argv, specs};

    if (!args.parse()) {
        std::println(stderr, "mousemove: {}", args.error());
        return 1;
    }

    if (args.has("-h", "--help")) {
        std::print("{}", help::mousemove);
        return 0;
    }

    const bool absolute = args.has("-a", "--absolute");
    const bool wheel = args.has("-w", "--wheel");

    std::array<std::optional<int>, 2> coordinates{
        args.int_value("-x", "--xpos"),
        args.int_value("-y", "--ypos"),
    };
    if (!args.ok()) {
        std::println(stderr, "mousemove: {}", args.error());
        return 1;
    }

    for (const auto& text : args.positional()) {
        const std::size_t index = coordinates[0] ? 1U : 0U;
        if (coordinates[index]) {
            std::println(stderr, "mousemove: too many coordinates");
            return 1;
        }

        const auto value = parse_int(text);
        if (!value) {
            std::println(stderr, "mousemove: invalid coordinate '{}'", text);
            return 1;
        }
        coordinates[index] = *value;
    }

    if (!coordinates[0] && !coordinates[1] && args.positional().empty()) {
        std::print("{}", help::mousemove);
        return 0;
    }
    if (!coordinates[0] || !coordinates[1]) {
        std::println(stderr, "mousemove: both x and y are required");
        return 1;
    }

    if (absolute && wheel) {
        std::println(stderr, "mousemove: the wheel cannot be absolute");
        return 1;
    }

    const int x = *coordinates[0];
    const int y = *coordinates[1];

    if (absolute) {
        if (!client.emit(EV_REL, REL_X, absolute_reset, false) ||
            !client.emit(EV_REL, REL_Y, absolute_reset, true)) {
            return 1;
        }
    }

    if (wheel) {
        if (!client.emit(EV_REL, REL_HWHEEL, x, false) ||
            !client.emit(EV_REL, REL_WHEEL, y, true)) {
            return 1;
        }
    } else {
        if (!client.emit(EV_REL, REL_X, x, false) ||
            !client.emit(EV_REL, REL_Y, y, true)) {
            return 1;
        }
    }

    return 0;
}
