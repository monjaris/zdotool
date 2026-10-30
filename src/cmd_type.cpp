#include "zdotool.hpp"

#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <optional>
#include <print>
#include <string>
#include <string_view>

namespace {

using zdotool::KeyDef;
using zdotool::ascii_typing_map;
using zdotool::emit_key;
using zdotool::lookup;
using zdotool::sleep_ms;

constexpr std::array specs{
    zdotool::OptSpec{"-d", "--key-delay", true},
    zdotool::OptSpec{"-H", "--key-hold", true},
    zdotool::OptSpec{"-D", "--next-delay", true},
    zdotool::OptSpec{"-f", "--file", true},
    zdotool::OptSpec{"-e", "--escape", true},
    zdotool::OptSpec{"-h", "--help", false},
};

// Decodes the escape sequences supported by `type`: \n, \t, \\ and \xHH.
class Escaper {
public:
    std::optional<char> next(char input) {
        switch (state_) {
        case State::plain:
            if (input == '\\') {
                state_ = State::escape;
                return std::nullopt;
            }
            return input;

        case State::escape:
            state_ = State::plain;
            switch (input) {
            case 'n':
                return '\n';
            case 't':
                return '\t';
            case '\\':
                return '\\';
            case 'x':
                state_ = State::hex_first;
                return std::nullopt;
            default:
                return std::nullopt;
            }

        case State::hex_first:
            state_ = State::hex_second;
            hex_[0] = input;
            return std::nullopt;

        case State::hex_second: {
            state_ = State::plain;
            hex_[1] = input;
            const auto value = zdotool::parse_int(std::string_view{hex_.data(), hex_.size()}, 16);
            if (!value) {
                return std::nullopt;
            }
            return static_cast<char>(*value);
        }
        }

        return std::nullopt;
    }

private:
    enum class State { plain, escape, hex_first, hex_second };

    State state_ = State::plain;
    std::array<char, 2> hex_{};
};

[[nodiscard]] bool type_char(zdotool::Client& client, char character, int hold_ms) {
    const KeyDef key = lookup(ascii_typing_map, static_cast<unsigned char>(character));
    return emit_key(client, key, hold_ms);
}

[[nodiscard]] bool type_string(zdotool::Client& client, std::string_view text, bool escape,
                               int key_delay_ms, int key_hold_ms) {
    Escaper escaper;
    bool first = true;

    for (const char raw : text) {
        const auto decoded = escape ? escaper.next(raw) : std::optional<char>{raw};
        if (!decoded) {
            continue;
        }
        if (!first) {
            sleep_ms(key_delay_ms);
        }
        if (!type_char(client, *decoded, key_hold_ms)) {
            return false;
        }
        first = false;
    }

    return true;
}

bool type_stream(zdotool::Client& client, std::istream& input, bool escape, int key_delay_ms,
                 int key_hold_ms) {
    Escaper escaper;
    char raw = 0;

    while (input.get(raw)) {
        const auto decoded = escape ? escaper.next(raw) : std::optional<char>{raw};
        if (!decoded) {
            continue;
        }
        if (!type_char(client, *decoded, key_hold_ms)) {
            break;
        }
        sleep_ms(key_delay_ms);
    }

    return !input.bad();
}

} // namespace

int zdotool::cmd_type(Client& client, int argc, char** argv) {
    Args args{argc, argv, specs};

    if (!args.parse()) {
        std::println(stderr, "type: {}", args.error());
        return 1;
    }

    const bool help_requested = args.has("-h", "--help");
    const auto file_path = args.value("-f", "--file");

    if (help_requested || (!file_path && args.positional().empty())) {
        std::print("{}", help::type_text());
        return 0;
    }

    const int key_delay_ms = args.get_int("-d", "--key-delay", default_key_delay_ms);
    const int key_hold_ms = args.get_int("-H", "--key-hold", default_key_hold_ms);
    const int next_delay_ms = args.get_int("-D", "--next-delay", default_next_delay_ms);
    const auto escape_option = args.int_value("-e", "--escape");
    if (!args.ok()) {
        std::println(stderr, "type: {}", args.error());
        return 1;
    }
    if (key_delay_ms < 0 || key_hold_ms < 0 || next_delay_ms < 0) {
        std::println(stderr, "type: delays must be non-negative");
        return 1;
    }

    // Escape sequences are enabled for command-line strings and disabled for
    // files.
    const bool escape = escape_option ? (*escape_option != 0) : !file_path.has_value();

    if (file_path) {
        std::ifstream file;
        std::istream* input = nullptr;

        if (*file_path == "-") {
            input = &std::cin;
        } else {
            file.open(std::string{*file_path}, std::ios::binary);
            if (!file) {
                std::println(stderr, "type: failed to open '{}'", *file_path);
                return 2;
            }
            input = &file;
        }

        if (!type_stream(client, *input, escape, key_delay_ms, key_hold_ms)) {
            std::println(stderr, "type: failed to read '{}'", *file_path);
            return 2;
        }
        return 0;
    }

    const auto strings = args.positional();
    for (std::size_t i = 0; i < strings.size(); ++i) {
        if (!type_string(client, strings[i], escape, key_delay_ms, key_hold_ms)) {
            return 1;
        }
        if (i + 1 < strings.size()) {
            sleep_ms(next_delay_ms);
        }
    }

    return 0;
}
