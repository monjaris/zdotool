#include "zdotool.hpp"

#include <algorithm>
#include <array>
#include <print>
#include <string_view>

namespace {

struct Command {
    std::string_view name;
    std::string_view summary;
    zdotool::CommandFn run; // nullptr for commands that do not use the client
};

constexpr std::array<Command, 6> commands{{
    {"click", "Click mouse buttons", zdotool::cmd_click},
    {"daemon", "Run the input daemon", nullptr},
    {"key", "Press keys by keycode", zdotool::cmd_key},
    {"mousemove", "Move the mouse pointer or wheel", zdotool::cmd_mousemove},
    {"stdin", "Forward terminal input as key events", zdotool::cmd_stdin},
    {"type", "Type strings", zdotool::cmd_type},
}};

void print_usage() {
    std::println("Usage: {} <command> [options] [args]\n", zdotool::program);
    std::println("Available commands:");
    for (const auto& command : commands) {
        std::println("  {:<10} {}", command.name, command.summary);
    }
    std::println();
    std::println("Environment:");
    std::println("  {}  Path to the daemon socket", zdotool::socket_env);
    std::println();
    std::println("Run '{} <command> --help' for command-specific options.", zdotool::program);
}

bool is_help(std::string_view argument) {
    return argument == "-h" || argument == "--help" || argument == "help";
}

bool is_version(std::string_view argument) {
    return argument == "-V" || argument == "--version";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 0;
    }

    const std::string_view first{argv[1]};

    if (is_help(first)) {
        print_usage();
        return 0;
    }

    if (is_version(first)) {
        std::println("{} {}", zdotool::program, zdotool::version);
        return 0;
    }

    const auto command = std::ranges::find(commands, first, &Command::name);
    if (command == commands.end()) {
        std::println(stderr, "{}: unknown command '{}'", zdotool::program, first);
        std::println(stderr, "Run '{} help' for a list of commands.", zdotool::program);
        return 1;
    }

    if (command->run == nullptr) {
        return zdotool::cmd_daemon(argc - 1, argv + 1);
    }

    zdotool::Client client{zdotool::socket_path()};
    const int status = command->run(client, argc - 1, argv + 1);

    return client.ok() ? status : 2;
}
