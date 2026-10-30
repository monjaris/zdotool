#pragma once

#include <charconv>
#include <chrono>
#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include <unistd.h>

#include <linux/input-event-codes.h>
#include <linux/uinput.h>

namespace zdotool {

inline constexpr std::string_view version = "0.1.0";
inline constexpr std::string_view program = "zdotool";
inline constexpr std::string_view daemon_program = "zdotool daemon";

inline constexpr std::string_view socket_env = "ZDOTOOL_SOCKET";
inline constexpr std::string_view runtime_dir_env = "XDG_RUNTIME_DIR";
inline constexpr std::string_view socket_file = ".zdotool_socket";
inline constexpr std::string_view default_socket = "/tmp/.zdotool_socket";

inline constexpr int default_click_repeat = 1;
inline constexpr int default_click_delay_ms = 25;
inline constexpr int default_key_command_delay_ms = 12;
inline constexpr int default_key_delay_ms = 20;
inline constexpr int default_key_hold_ms = 20;
inline constexpr int default_next_delay_ms = 0;

// Small RAII wrapper around a file descriptor. Move-only; closes on destruction.
class Fd {
public:
    Fd() = default;
    explicit Fd(int fd) noexcept : fd_(fd) {}
    ~Fd() { reset(); }

    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;

    Fd(Fd&& other) noexcept : fd_(std::exchange(other.fd_, -1)) {}

    Fd& operator=(Fd&& other) noexcept {
        if (this != &other) {
            reset();
            fd_ = std::exchange(other.fd_, -1);
        }
        return *this;
    }

    [[nodiscard]] int get() const noexcept { return fd_; }
    [[nodiscard]] bool valid() const noexcept { return fd_ >= 0; }
    explicit operator bool() const noexcept { return valid(); }

    void reset(int fd = -1) noexcept {
        if (fd_ >= 0) {
            ::close(fd_);
        }
        fd_ = fd;
    }

private:
    int fd_ = -1;
};

inline void sleep_ms(int milliseconds) {
    if (milliseconds > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds{milliseconds});
    }
}

inline std::optional<int> parse_int(std::string_view text, int base = 10) {
    int value = 0;
    const char* first = text.data();
    const char* last = first + text.size();
    const auto [ptr, ec] = std::from_chars(first, last, value, base);
    if (ec != std::errc{} || ptr != last) {
        return std::nullopt;
    }
    return value;
}

// Resolve the daemon socket path from the environment.
inline std::string socket_path() {
    if (const char* path = std::getenv(socket_env.data()); path != nullptr && *path != '\0') {
        return path;
    }
    if (const char* runtime_dir = std::getenv(runtime_dir_env.data());
        runtime_dir != nullptr && *runtime_dir != '\0') {
        return std::string{runtime_dir} + '/' + std::string{socket_file};
    }
    return std::string{default_socket};
}

} // namespace zdotool
