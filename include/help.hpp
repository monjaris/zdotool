#pragma once

#include "common.hpp"

#include <format>
#include <string>
#include <string_view>

namespace zdotool::help {

inline constexpr std::string_view click = R"(Usage: zdotool click [OPTION]... [BUTTONS]...

Click mouse buttons.

Options:
  -r, --repeat=N             Repeat the sequence N times (default: 1)
  -D, --next-delay=N         Delay N milliseconds between events (default: 25)
  -P, --press-release        Press and release in a single step
  -h, --help                 Display this help and exit

Buttons can be names or hexadecimal values with an optional bit mask:
  L/LEFT, R/RIGHT, M/MIDDLE, X/EXTRA
  S/SIDE, F/FORWARD, B/BACK, T/TASK
  Names perform a full click.

Numeric values:
  0x00 LEFT, 0x01 RIGHT, 0x02 MIDDLE, 0x03 SIDE
  0x04 EXTRA, 0x05 FORWARD, 0x06 BACK, 0x07 TASK
  0x40 press only, 0x80 release only, 0xC0 press and release
  0x00 does nothing but still waits for --next-delay

Examples:
  L      left click
  R      right click
  0xC0   left click
  0x41   right button down
  0x82   middle button up
)";

inline constexpr std::string_view daemon = R"(Usage: zdotool daemon [OPTION]...

Run the zdotool input daemon. It holds a persistent uinput device and forwards
events received over the socket to it.

Options:
  -p, --socket-path=PATH     Custom socket path
  -P, --socket-perm=PERM     Socket permission (default 0600)
  -o, --socket-own=UID:GID   Socket ownership
  -m, --mouse-off            Disable mouse (EV_REL)
  -k, --keyboard-off         Disable keyboard (EV_KEY)
  -T, --touch-on             Enable touchscreen (EV_ABS)
  -h, --help                 Display this help and exit
  -V, --version              Show version information
)";

inline constexpr std::string_view key = R"(Usage: zdotool key [OPTION]... [KEYCODES]...

Emit raw key events.

Options:
  -d, --key-delay=N          Delay N milliseconds between key events (default: 12)
  -h, --help                 Display this help and exit

Each KEYCODE is <code>[:<pressed>]:
  <code> is decimal, or hexadecimal when prefixed with 0x
  <pressed> is 1 for a key press (the default) and 0 for a release

Examples:
  28:1 28:0                                  press and release Enter
  42:1 38:1 38:0 24:1 24:0 38:1 38:0 42:0   type "LOL" while holding Shift

See /usr/include/linux/input-event-codes.h for key codes.
)";

inline constexpr std::string_view mousemove = R"(Usage: zdotool mousemove [OPTION]... [-x <xpos> -y <ypos>] [-- <xpos> <ypos>]

Move the mouse pointer or the wheel.

Options:
  -w, --wheel                Move the mouse wheel relatively
  -a, --absolute             Use absolute positioning (not applicable to the wheel)
  -x, --xpos                 X position
  -y, --ypos                 Y position
  -h, --help                 Display this help and exit

Mouse acceleration must be disabled for correct absolute movement.
)";

inline std::string type_text() {
    return std::format(
        "Usage: zdotool type [OPTION]... [STRINGS]...\n"
        "\n"
        "Type strings.\n"
        "\n"
        "Options:\n"
        "  -d, --key-delay=N          Delay N milliseconds between keys (default: {})\n"
        "  -H, --key-hold=N           Hold each key for N milliseconds (default: {})\n"
        "  -D, --next-delay=N         Delay N milliseconds between strings (default: {})\n"
        "  -f, --file=PATH            Type the contents of PATH ('-' reads stdin)\n"
        "  -e, --escape=BOOL          Enable (1) or disable (0) escape sequences\n"
        "  -h, --help                 Display this help and exit\n"
        "\n"
        "Escape sequences are enabled by default for command-line strings and\n"
        "disabled by default when reading from a file or stdin.\n"
        "\n"
        "Supported escapes: \\n, \\t, \\\\ and \\xHH.\n",
        default_key_delay_ms, default_key_hold_ms, default_next_delay_ms);
}

inline constexpr std::string_view stdin_forward = R"(Usage: zdotool stdin [OPTION]...

Forward input read from the terminal to the daemon as key events.
The terminal is switched to raw mode; press Ctrl-C to stop.

Options:
  -h, --help                 Display this help and exit
)";

} // namespace zdotool::help
