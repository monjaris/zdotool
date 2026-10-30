#include "client.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <print>
#include <utility>

#include <sys/socket.h>
#include <sys/un.h>

namespace zdotool {

bool Client::connect() {
    if (fd_.valid()) {
        return true;
    }
    if (error_ != 0) {
        return false;
    }

    Fd socket{::socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0)};
    if (!socket.valid()) {
        error_ = errno;
        report_error();
        return false;
    }

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    if (path_.size() + 1 > sizeof(address.sun_path)) {
        error_ = ENAMETOOLONG;
        report_error();
        return false;
    }
    std::snprintf(address.sun_path, sizeof(address.sun_path), "%s", path_.c_str());

    if (::connect(socket.get(), reinterpret_cast<const sockaddr*>(&address), sizeof(address)) !=
        0) {
        error_ = errno;
        report_error();
        return false;
    }

    fd_ = std::move(socket);
    return true;
}

bool Client::write_event(const input_event& event) {
    ssize_t sent;
    do {
        sent = ::send(fd_.get(), &event, sizeof(event), 0);
    } while (sent < 0 && errno == EINTR);

    if (sent != static_cast<ssize_t>(sizeof(event))) {
        error_ = sent < 0 ? errno : EIO;
        report_error();
        return false;
    }
    return true;
}

bool Client::emit(std::uint16_t type, std::uint16_t code, std::int32_t value, bool syn) {
    if (!connect()) {
        return false;
    }

    input_event event{};
    event.type = type;
    event.code = code;
    event.value = value;

    if (!write_event(event)) {
        return false;
    }
    if (!syn) {
        return true;
    }

    input_event syn_event{};
    syn_event.type = EV_SYN;
    syn_event.code = SYN_REPORT;
    return write_event(syn_event);
}

void Client::report_error() {
    if (reported_) {
        return;
    }
    reported_ = true;

    std::println(stderr, "{}: failed to {} '{}': {}", program,
                 fd_.valid() ? "send to" : "connect to", path_, std::strerror(error_));

    switch (error_) {
    case ENOENT:
    case ECONNREFUSED:
        std::println(stderr, "Please check if '{}' is running.", daemon_program);
        break;
    case EACCES:
    case EPERM:
        std::println(stderr, "Please check if the current user can access '{}'.", path_);
        break;
    default:
        break;
    }
}

} // namespace zdotool
