#include "zdotool.hpp"

#include <array>
#include <cerrno>
#include <csignal>
#include <optional>
#include <poll.h>
#include <print>

#include <termios.h>
#include <unistd.h>

namespace {

using zdotool::K;
using zdotool::KeyDef;

constexpr std::array specs{
    zdotool::OptSpec{"-h", "--help", false},
};

std::optional<KeyDef> terminal_key(char final) {
    switch (final) {
    case 'A':
        return K(KEY_UP);
    case 'B':
        return K(KEY_DOWN);
    case 'C':
        return K(KEY_RIGHT);
    case 'D':
        return K(KEY_LEFT);
    case 'I':
        return K(KEY_PAGEUP);
    case 'J':
        return K(KEY_PAGEDOWN);
    default:
        return std::nullopt;
    }
}

std::optional<termios> saved_terminal;

void restore_terminal() {
    if (saved_terminal.has_value()) {
        ::tcsetattr(STDIN_FILENO, TCSANOW, &*saved_terminal);
        saved_terminal.reset();
    }
}

void on_signal(int) {
    restore_terminal();
    ::_exit(0);
}

class RawTerminal {
public:
    RawTerminal() {
        termios current{};
        if (::tcgetattr(STDIN_FILENO, &current) != 0) {
            return;
        }

        saved_terminal = current;

        termios raw = current;
        raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;

        if (::tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
            saved_terminal.reset();
            return;
        }

        ::atexit(restore_terminal);
        std::signal(SIGINT, on_signal);
        active_ = true;
    }

    ~RawTerminal() {
        if (active_) {
            restore_terminal();
        }
    }

    RawTerminal(const RawTerminal&) = delete;
    RawTerminal& operator=(const RawTerminal&) = delete;

private:
    bool active_ = false;
};

// Read one byte. A negative timeout blocks; zero or more waits that many ms.
bool read_byte(char& byte, int timeout_ms) {
    if (timeout_ms >= 0) {
        pollfd descriptor{STDIN_FILENO, POLLIN, 0};
        int ready = 0;
        do {
            ready = ::poll(&descriptor, 1, timeout_ms);
        } while (ready < 0 && errno == EINTR);

        if (ready <= 0) {
            return false;
        }
    }

    ssize_t count = 0;
    do {
        count = ::read(STDIN_FILENO, &byte, 1);
    } while (count < 0 && errno == EINTR);

    return count == 1;
}

} // namespace

int zdotool::cmd_stdin(Client& client, int argc, char** argv) {
    Args args{argc, argv, specs};

    if (!args.parse()) {
        std::println(stderr, "stdin: {}", args.error());
        return 1;
    }

    if (args.has("-h", "--help")) {
        std::print("{}", help::stdin_forward);
        return 0;
    }

    if (!args.positional().empty()) {
        std::println(stderr, "stdin: this command takes no arguments");
        return 1;
    }

    RawTerminal terminal;
    std::println("Forwarding terminal input to {} (Ctrl-C to stop).", daemon_program);

    constexpr int hold_ms = 20;

    while (true) {
        char byte = 0;
        if (!read_byte(byte, -1)) {
            break;
        }

        if (byte == '\x1b') {
            char first = 0;
            char second = 0;
            if (read_byte(first, 10) && first == '[' && read_byte(second, 10)) {
                if (const auto key = terminal_key(second)) {
                    if (!emit_key(client, *key, hold_ms)) {
                        break;
                    }
                    continue;
                }
            }
            if (!emit_key(client, K(KEY_ESC), hold_ms)) {
                break;
            }
            continue;
        }

        if (!emit_key(client, lookup(ascii_stdin_map, static_cast<unsigned char>(byte)), hold_ms)) {
            break;
        }
        sleep_ms(default_key_delay_ms);
    }

    return 0;
}
