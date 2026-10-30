#pragma once

#define HELP_MSG constexpr const char*

namespace help
{

    HELP_MSG click = {
        "Usage: click [OPTION]... [BUTTONS]...\n"
        "Click mouse buttons.\n"
        "\n"
        "Options:\n"
        "  -r, --repeat=N             Repeat entire sequence N times\n"
        "  -D, --next-delay=N         Delay N milliseconds between input events (up/down, "
        "                               a complete click means doubled time)\n"
        "  -P, --press-release        Press & release mouse button to complete a click, (optional)\n"
        "  -h, --help                 Display this help and exit\n"
        "\n"
        "How to specify buttons:\n"
        "  Now all mouse buttons are represented using hexadecimal numeric values, with an optional\n"
        "bit mask to specify if mouse up/down needs to be omitted.\n"
        "  0x00 - LEFT\n"
        "  0x01 - RIGHT\n"
        "  0x02 - MIDDLE\n"
        "  0x03 - SIDE\n"
        "  0x04 - EXTR\n"
        "  0x05 - FORWARD\n"
        "  0x06 - BACK\n"
        "  0x07 - TASK\n"
        "  0x40 - Mouse down\n"
        "  0x80 - Mouse up\n"
        "  Examples:\n"
        "    0x00: chooses left button, but does nothing (you can use this to implement extra sleeps)\n"
        "    0xC0: left button click (down then up)\n"
        "    0x41: right button down\n"
        "    0x82: middle button up\n"
        "  The '0x' prefix can be omitted if you want.\n"
    };



    HELP_MSG daemon = {
        "Usage: ydotoold [OPTION]...\n"
        "The ydotool Daemon.\n"
        "\n"
        "Options:\n"
        "  -p, --socket-path=PATH     Custom socket path\n"
        "  -P, --socket-perm=PERM     Socket permission (default 0600)\n"
        "  -o, --socket-own=UID:GID   Socket ownership\n"
        "  -m, --mouse-off            Disable mouse (EV_REL)\n"
        "  -k, --keyboard-off         Disable keyboard (EV_KEY)\n"
        "  -T, --touch-on             Enable touchscreen (EV_ABS)\n"
        "  -h, --help                 Display this help and exit\n"
        "  -V, --version              Show version information\n"
    };



    HELP_MSG key = {
        "Usage: key [OPTION]... [KEYCODES]...\n"
        "Emit key events.\n"
        "\n"
        "Options:\n"
        "  -d, --key-delay=N          Delay N milliseconds between key events\n"
        "  -h, --help                 Display this help and exit\n"
        "\n"
        "Since there's no way to know how many keyboard layouts are there in the world,\n"
        "we're using raw keycodes now.\n"
        "\n"
        "Syntax: <keycode>:<pressed>\n"
        "e.g. 28:1 28:0 means pressing on the Enter button on a standard US keyboard.\n"
        "     (where :1 for pressed means the key is down and then :0 means the key is released)"
        "     42:1 38:1 38:0 24:1 24:0 38:1 38:0 42:0 - \"LOL\"\n"
        "\n"
        "Non-interpretable values, such as 0, aaa, l0l, will only cause a delay.\n"
        "\n"
        "See `/usr/include/linux/input-event-codes.h' for available key codes (KEY_*).\n"
    };



    HELP_MSG mousemove = {
        "Usage: mousemove [OPTION]... [-x <xpos> -y <ypos>] [-- <xpos> <ypos>]\n"
        "Move mouse pointer or wheel.\n"
        "\n"
        "Options:\n"
        "  -w, --wheel                Move mouse wheel relatively\n"
        "  -a, --absolute             Use absolute position, not applicable to wheel\n"
        "  -x, --xpos                 X position\n"
        "  -y, --ypos                 Y position\n"
        "  -h, --help                 Display this help and exit\n"
        "\n"
        "You need to disable mouse speed acceleration for correct absolute movement."
    };


    HELP_MSG type = {
		"Usage: type [OPTION]... [STRINGS]...\n"
		"Type strings.\n"
		"\n"
		"Options:"
		"  -d, --key-delay=N          Delay N milliseconds between keys (the delay between every key down/up pair) (default: %d)\n {opt_key_delay_ms}"
		"  -H, --key-hold=N           Hold each key for N milliseconds (the delay between key down and up) (default: %d)\n {opt_key_hold_ms}"
		"  -D, --next-delay=N         Delay N milliseconds between command line strings (default: %d)\n {opt_next_delay_ms}"
		"  -f, --file=PATH            Specify a file, the contents of which will be be typed as if passed as an argument.\n"
		"                               The filepath may also be '-' to read from stdin\n"
		"  -e, --escape=BOOL          Escape enable (1) or disable (0)\n"
		"  -h, --help                 Display this help and exit\n"
		"\n"
		"Escape is enabled by default when typing command line arguments, and disabled by default when typing from file and stdin."
    };
}


#undef HELP_MSG
