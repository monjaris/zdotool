#pragma once

#include "common.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace zdotool {

// Connection to the zdotool daemon. The socket is opened lazily, on the first
// emit(), so `--help` and argument errors work without a running daemon.
class Client {
public:
    explicit Client(std::string path) : path_(std::move(path)) {}

    [[nodiscard]] bool emit(std::uint16_t type, std::uint16_t code, std::int32_t value,
                            bool syn = true);
    [[nodiscard]] bool ok() const noexcept { return error_ == 0; }

private:
    bool connect();
    bool write_event(const input_event& event);
    void report_error();

    std::string path_;
    Fd fd_;
    int error_ = 0;
    bool reported_ = false;
};

} // namespace zdotool
