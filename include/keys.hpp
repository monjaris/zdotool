#pragma once

#include "client.hpp"
#include "keymap.hpp"

namespace zdotool {

// Emit a single key stroke, including its modifiers, and hold it for `hold_ms`.
// Returns false only when the event could not be sent.
[[nodiscard]] inline bool emit_key(Client& client, const KeyDef& key, int hold_ms) {
    if (!key.valid()) {
        return true;
    }
    if (key.shift && !client.emit(EV_KEY, KEY_LEFTSHIFT, 1, true)) {
        return false;
    }
    if (key.ctrl && !client.emit(EV_KEY, KEY_LEFTCTRL, 1, true)) {
        return false;
    }

    if (!client.emit(EV_KEY, key.code, 1, true)) {
        return false;
    }
    sleep_ms(hold_ms);
    if (!client.emit(EV_KEY, key.code, 0, true)) {
        return false;
    }

    if (key.ctrl && !client.emit(EV_KEY, KEY_LEFTCTRL, 0, true)) {
        return false;
    }
    if (key.shift && !client.emit(EV_KEY, KEY_LEFTSHIFT, 0, true)) {
        return false;
    }
    return true;
}

} // namespace zdotool
