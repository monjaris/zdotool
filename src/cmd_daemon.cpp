#include "zdotool.hpp"

#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <print>
#include <string>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

namespace {

constexpr std::size_t socket_path_max = sizeof(sockaddr_un::sun_path);
constexpr const char* device_name = "ydotoold virtual device";

struct UinputSetup {
    bool keyboard = true;
    bool mouse = true;
    bool touch = false;
};

struct DaemonOptions {
    std::string socket_path = zdotool::socket_path();
    std::string socket_perm = "0600";
    std::string socket_own;
    UinputSetup setup;
};

constexpr std::array specs{
    zdotool::OptSpec{"-p", "--socket-path", true},
    zdotool::OptSpec{"-P", "--socket-perm", true},
    zdotool::OptSpec{"-o", "--socket-own", true},
    zdotool::OptSpec{"-m", "--mouse-off", false},
    zdotool::OptSpec{"-k", "--keyboard-off", false},
    zdotool::OptSpec{"-T", "--touch-on", false},
    zdotool::OptSpec{"-h", "--help", false},
    zdotool::OptSpec{"-V", "--version", false},
};

sockaddr_un make_address(const std::string& path) {
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::snprintf(address.sun_path, sizeof(address.sun_path), "%s", path.c_str());
    return address;
}

bool configure_device(zdotool::Fd& fd, const UinputSetup& setup) {
    if (setup.keyboard) {
        if (::ioctl(fd.get(), UI_SET_EVBIT, EV_KEY) < 0) {
            std::println(stderr, "{}: UI_SET_EVBIT EV_KEY failed", zdotool::daemon_program);
        }

        constexpr std::array key_list = {
            KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7,
            KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_TAB, KEY_Q,
            KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O,
            KEY_P, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_ENTER, KEY_LEFTCTRL, KEY_A, KEY_S, KEY_D,
            KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_APOSTROPHE,
            KEY_GRAVE, KEY_LEFTSHIFT, KEY_BACKSLASH, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B,
            KEY_N, KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_RIGHTSHIFT, KEY_KPASTERISK, KEY_LEFTALT,
            KEY_SPACE, KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
            KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUMLOCK, KEY_SCROLLLOCK, KEY_KP7, KEY_KP8,
            KEY_KP9, KEY_KPMINUS, KEY_KP4, KEY_KP5, KEY_KP6, KEY_KPPLUS, KEY_KP1, KEY_KP2,
            KEY_KP3, KEY_KP0, KEY_KPDOT, KEY_ZENKAKUHANKAKU, KEY_102ND, KEY_F11, KEY_F12, KEY_RO,
            KEY_KATAKANA, KEY_HIRAGANA, KEY_HENKAN, KEY_KATAKANAHIRAGANA, KEY_MUHENKAN, KEY_KPJPCOMMA, KEY_KPENTER, KEY_RIGHTCTRL,
            KEY_KPSLASH, KEY_SYSRQ, KEY_RIGHTALT, KEY_LINEFEED, KEY_HOME, KEY_UP, KEY_PAGEUP, KEY_LEFT,
            KEY_RIGHT, KEY_END, KEY_DOWN, KEY_PAGEDOWN, KEY_INSERT, KEY_DELETE, KEY_MACRO, KEY_MUTE,
            KEY_VOLUMEDOWN, KEY_VOLUMEUP, KEY_POWER, KEY_KPEQUAL, KEY_KPPLUSMINUS, KEY_PAUSE, KEY_SCALE, KEY_KPCOMMA,
            KEY_HANGEUL, KEY_HANGUEL, KEY_HANJA, KEY_YEN, KEY_LEFTMETA, KEY_RIGHTMETA, KEY_COMPOSE, KEY_STOP,
            KEY_AGAIN, KEY_PROPS, KEY_UNDO, KEY_FRONT, KEY_COPY, KEY_OPEN, KEY_PASTE, KEY_FIND,
            KEY_CUT, KEY_HELP, KEY_MENU, KEY_CALC, KEY_SETUP, KEY_SLEEP, KEY_WAKEUP, KEY_FILE,
            KEY_SENDFILE, KEY_DELETEFILE, KEY_XFER, KEY_PROG1, KEY_PROG2, KEY_WWW, KEY_MSDOS, KEY_COFFEE,
            KEY_SCREENLOCK, KEY_ROTATE_DISPLAY, KEY_DIRECTION, KEY_CYCLEWINDOWS, KEY_MAIL, KEY_BOOKMARKS, KEY_COMPUTER, KEY_BACK,
            KEY_FORWARD, KEY_CLOSECD, KEY_EJECTCD, KEY_EJECTCLOSECD, KEY_NEXTSONG, KEY_PLAYPAUSE, KEY_PREVIOUSSONG, KEY_STOPCD,
            KEY_RECORD, KEY_REWIND, KEY_PHONE, KEY_ISO, KEY_CONFIG, KEY_HOMEPAGE, KEY_REFRESH, KEY_EXIT,
            KEY_MOVE, KEY_EDIT, KEY_SCROLLUP, KEY_SCROLLDOWN, KEY_KPLEFTPAREN, KEY_KPRIGHTPAREN, KEY_NEW, KEY_REDO,
            KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20,
            KEY_F21, KEY_F22, KEY_F23, KEY_F24, KEY_PLAYCD, KEY_PAUSECD, KEY_PROG3, KEY_PROG4,
            KEY_DASHBOARD, KEY_SUSPEND, KEY_CLOSE, KEY_PLAY, KEY_FASTFORWARD, KEY_BASSBOOST, KEY_PRINT, KEY_HP,
            KEY_CAMERA, KEY_SOUND, KEY_QUESTION, KEY_EMAIL, KEY_CHAT, KEY_SEARCH, KEY_CONNECT, KEY_FINANCE,
            KEY_SPORT, KEY_SHOP, KEY_ALTERASE, KEY_CANCEL, KEY_BRIGHTNESSDOWN, KEY_BRIGHTNESSUP, KEY_MEDIA, KEY_SWITCHVIDEOMODE,
            KEY_KBDILLUMTOGGLE, KEY_KBDILLUMDOWN, KEY_KBDILLUMUP, KEY_SEND, KEY_REPLY, KEY_FORWARDMAIL, KEY_SAVE, KEY_DOCUMENTS,
            KEY_BATTERY, KEY_BLUETOOTH, KEY_WLAN, KEY_UWB, KEY_UNKNOWN, KEY_VIDEO_NEXT, KEY_VIDEO_PREV, KEY_BRIGHTNESS_CYCLE,
            KEY_BRIGHTNESS_AUTO, KEY_BRIGHTNESS_ZERO, KEY_DISPLAY_OFF, KEY_WWAN, KEY_WIMAX, KEY_RFKILL, KEY_MICMUTE, KEY_OK,
            KEY_SELECT, KEY_GOTO, KEY_CLEAR, KEY_POWER2, KEY_OPTION, KEY_INFO, KEY_TIME, KEY_VENDOR,
            KEY_ARCHIVE, KEY_PROGRAM, KEY_CHANNEL, KEY_FAVORITES, KEY_EPG, KEY_PVR, KEY_MHP, KEY_LANGUAGE,
            KEY_TITLE, KEY_SUBTITLE, KEY_ANGLE, KEY_ZOOM, KEY_MODE, KEY_KEYBOARD, KEY_SCREEN, KEY_PC,
            KEY_TV, KEY_TV2, KEY_VCR, KEY_VCR2, KEY_SAT, KEY_SAT2, KEY_CD, KEY_TAPE,
            KEY_RADIO, KEY_TUNER, KEY_PLAYER, KEY_TEXT, KEY_DVD, KEY_AUX, KEY_MP3, KEY_AUDIO,
            KEY_VIDEO, KEY_DIRECTORY, KEY_LIST, KEY_MEMO, KEY_CALENDAR, KEY_RED, KEY_GREEN, KEY_YELLOW,
            KEY_BLUE, KEY_CHANNELUP, KEY_CHANNELDOWN, KEY_FIRST, KEY_LAST, KEY_AB, KEY_NEXT, KEY_RESTART,
            KEY_SLOW, KEY_SHUFFLE, KEY_BREAK, KEY_PREVIOUS, KEY_DIGITS, KEY_TEEN, KEY_TWEN, KEY_VIDEOPHONE,
            KEY_GAMES, KEY_ZOOMIN, KEY_ZOOMOUT, KEY_ZOOMRESET, KEY_WORDPROCESSOR, KEY_EDITOR, KEY_SPREADSHEET, KEY_GRAPHICSEDITOR,
            KEY_PRESENTATION, KEY_DATABASE, KEY_NEWS, KEY_VOICEMAIL, KEY_ADDRESSBOOK, KEY_MESSENGER, KEY_DISPLAYTOGGLE, KEY_BRIGHTNESS_TOGGLE,
            KEY_SPELLCHECK, KEY_LOGOFF, KEY_DOLLAR, KEY_EURO, KEY_FRAMEBACK, KEY_FRAMEFORWARD, KEY_CONTEXT_MENU, KEY_MEDIA_REPEAT,
            KEY_10CHANNELSUP, KEY_10CHANNELSDOWN, KEY_IMAGES, KEY_DEL_EOL, KEY_DEL_EOS, KEY_INS_LINE, KEY_DEL_LINE, KEY_FN,
            KEY_FN_ESC, KEY_FN_F1, KEY_FN_F2, KEY_FN_F3, KEY_FN_F4, KEY_FN_F5, KEY_FN_F6, KEY_FN_F7,
            KEY_FN_F8, KEY_FN_F9, KEY_FN_F10, KEY_FN_F11, KEY_FN_F12, KEY_FN_1, KEY_FN_2, KEY_FN_D,
            KEY_FN_E, KEY_FN_F, KEY_FN_S, KEY_FN_B, KEY_BRL_DOT1, KEY_BRL_DOT2, KEY_BRL_DOT3, KEY_BRL_DOT4,
            KEY_BRL_DOT5, KEY_BRL_DOT6, KEY_BRL_DOT7, KEY_BRL_DOT8, KEY_BRL_DOT9, KEY_BRL_DOT10, KEY_NUMERIC_0, KEY_NUMERIC_1,
            KEY_NUMERIC_2, KEY_NUMERIC_3, KEY_NUMERIC_4, KEY_NUMERIC_5, KEY_NUMERIC_6, KEY_NUMERIC_7, KEY_NUMERIC_8, KEY_NUMERIC_9,
            KEY_NUMERIC_STAR, KEY_NUMERIC_POUND, KEY_NUMERIC_A, KEY_NUMERIC_B, KEY_NUMERIC_C, KEY_NUMERIC_D, KEY_CAMERA_FOCUS, KEY_WPS_BUTTON,
            KEY_TOUCHPAD_TOGGLE, KEY_TOUCHPAD_ON, KEY_TOUCHPAD_OFF, KEY_CAMERA_ZOOMIN, KEY_CAMERA_ZOOMOUT, KEY_CAMERA_UP, KEY_CAMERA_DOWN, KEY_CAMERA_LEFT,
            KEY_CAMERA_RIGHT, KEY_ATTENDANT_ON, KEY_ATTENDANT_OFF, KEY_ATTENDANT_TOGGLE, KEY_LIGHTS_TOGGLE, KEY_ALS_TOGGLE, KEY_BUTTONCONFIG, KEY_TASKMANAGER,
            KEY_JOURNAL, KEY_CONTROLPANEL, KEY_APPSELECT, KEY_SCREENSAVER, KEY_VOICECOMMAND, KEY_BRIGHTNESS_MIN, KEY_BRIGHTNESS_MAX, KEY_KBDINPUTASSIST_PREV,
            KEY_KBDINPUTASSIST_NEXT, KEY_KBDINPUTASSIST_PREVGROUP, KEY_KBDINPUTASSIST_NEXTGROUP, KEY_KBDINPUTASSIST_ACCEPT, KEY_KBDINPUTASSIST_CANCEL, KEY_RIGHT_UP, KEY_RIGHT_DOWN, KEY_LEFT_UP,
            KEY_LEFT_DOWN, KEY_ROOT_MENU, KEY_MEDIA_TOP_MENU, KEY_NUMERIC_11, KEY_NUMERIC_12, KEY_AUDIO_DESC, KEY_3D_MODE, KEY_NEXT_FAVORITE,
            KEY_STOP_RECORD, KEY_PAUSE_RECORD, KEY_VOD, KEY_UNMUTE, KEY_FASTREVERSE, KEY_SLOWREVERSE, KEY_DATA, KEY_MIN_INTERESTING,
            BTN_MISC, BTN_0, BTN_1, BTN_2, BTN_3, BTN_4, BTN_5, BTN_6,
            BTN_7, BTN_8, BTN_9, BTN_MOUSE, BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, BTN_SIDE,
            BTN_EXTRA, BTN_FORWARD, BTN_BACK, BTN_TASK, BTN_JOYSTICK, BTN_TRIGGER, BTN_THUMB, BTN_THUMB2,
            BTN_TOP, BTN_TOP2, BTN_PINKIE, BTN_BASE, BTN_BASE2, BTN_BASE3, BTN_BASE4, BTN_BASE5,
            BTN_BASE6, BTN_DEAD, BTN_GAMEPAD, BTN_SOUTH, BTN_A, BTN_EAST, BTN_B, BTN_C,
            BTN_NORTH, BTN_X, BTN_WEST, BTN_Y, BTN_Z, BTN_TL, BTN_TR, BTN_TL2,
            BTN_TR2, BTN_SELECT, BTN_START, BTN_MODE, BTN_THUMBL, BTN_THUMBR, BTN_DIGI, BTN_TOOL_PEN,
            BTN_TOOL_RUBBER, BTN_TOOL_BRUSH, BTN_TOOL_PENCIL, BTN_TOOL_AIRBRUSH, BTN_TOOL_FINGER, BTN_TOOL_MOUSE, BTN_TOOL_LENS, BTN_TOOL_QUINTTAP,
            BTN_TOUCH, BTN_STYLUS, BTN_STYLUS2, BTN_TOOL_DOUBLETAP, BTN_TOOL_TRIPLETAP, BTN_TOOL_QUADTAP, BTN_WHEEL, BTN_GEAR_DOWN,
            BTN_GEAR_UP, BTN_DPAD_UP, BTN_DPAD_DOWN, BTN_DPAD_LEFT, BTN_DPAD_RIGHT, BTN_TRIGGER_HAPPY, BTN_TRIGGER_HAPPY1, BTN_TRIGGER_HAPPY2,
            BTN_TRIGGER_HAPPY3, BTN_TRIGGER_HAPPY4, BTN_TRIGGER_HAPPY5, BTN_TRIGGER_HAPPY6, BTN_TRIGGER_HAPPY7, BTN_TRIGGER_HAPPY8, BTN_TRIGGER_HAPPY9, BTN_TRIGGER_HAPPY10,
            BTN_TRIGGER_HAPPY11, BTN_TRIGGER_HAPPY12, BTN_TRIGGER_HAPPY13, BTN_TRIGGER_HAPPY14, BTN_TRIGGER_HAPPY15, BTN_TRIGGER_HAPPY16, BTN_TRIGGER_HAPPY17, BTN_TRIGGER_HAPPY18,
            BTN_TRIGGER_HAPPY19, BTN_TRIGGER_HAPPY20, BTN_TRIGGER_HAPPY21, BTN_TRIGGER_HAPPY22, BTN_TRIGGER_HAPPY23, BTN_TRIGGER_HAPPY24, BTN_TRIGGER_HAPPY25, BTN_TRIGGER_HAPPY26,
            BTN_TRIGGER_HAPPY27, BTN_TRIGGER_HAPPY28, BTN_TRIGGER_HAPPY29, BTN_TRIGGER_HAPPY30, BTN_TRIGGER_HAPPY31, BTN_TRIGGER_HAPPY32, BTN_TRIGGER_HAPPY33, BTN_TRIGGER_HAPPY34,
            BTN_TRIGGER_HAPPY35, BTN_TRIGGER_HAPPY36, BTN_TRIGGER_HAPPY37, BTN_TRIGGER_HAPPY38, BTN_TRIGGER_HAPPY39, BTN_TRIGGER_HAPPY40,
        };

        for (const int key : key_list) {
            if (::ioctl(fd.get(), UI_SET_KEYBIT, key) < 0) {
                std::println(stderr, "{}: UI_SET_KEYBIT {} failed", zdotool::daemon_program, key);
            }
        }
    }

    if (setup.mouse) {
        if (::ioctl(fd.get(), UI_SET_EVBIT, EV_REL) < 0) {
            std::println(stderr, "{}: UI_SET_EVBIT EV_REL failed", zdotool::daemon_program);
        }

        constexpr std::array rel_list{REL_X, REL_Y, REL_Z, REL_WHEEL, REL_HWHEEL};
        for (const int relative : rel_list) {
            if (::ioctl(fd.get(), UI_SET_RELBIT, relative) < 0) {
                std::println(stderr, "{}: UI_SET_RELBIT {} failed", zdotool::daemon_program,
                             relative);
            }
        }
    }

    if (setup.touch) {
        if (::ioctl(fd.get(), UI_SET_EVBIT, EV_ABS) < 0) {
            std::println(stderr, "{}: UI_SET_EVBIT EV_ABS failed", zdotool::daemon_program);
        }

        constexpr std::array abs_list{
            ABS_X,            ABS_Y,           ABS_MT_SLOT,          ABS_MT_TRACKING_ID,
            ABS_MT_POSITION_X, ABS_MT_POSITION_Y, ABS_PRESSURE,      ABS_MT_PRESSURE,
        };
        for (const int absolute : abs_list) {
            if (::ioctl(fd.get(), UI_SET_ABSBIT, absolute) < 0) {
                std::println(stderr, "{}: UI_SET_ABSBIT {} failed", zdotool::daemon_program,
                             absolute);
            }
        }
    }

    // Keep the upstream name so existing udev and compositor rules keep matching.
    uinput_setup device{};
    device.id.bustype = BUS_VIRTUAL;
    device.id.vendor = 0x2333;
    device.id.product = 0x6666;
    device.id.version = 1;
    std::snprintf(device.name, sizeof(device.name), "%s", device_name);

    if (::ioctl(fd.get(), UI_DEV_SETUP, &device) < 0) {
        std::println(stderr, "{}: UI_DEV_SETUP failed: {}", zdotool::daemon_program,
                     std::strerror(errno));
        return false;
    }

    if (::ioctl(fd.get(), UI_DEV_CREATE) < 0) {
        std::println(stderr, "{}: UI_DEV_CREATE failed: {}", zdotool::daemon_program,
                     std::strerror(errno));
        return false;
    }

    return true;
}

// Returns false if a live daemon is already using the socket.
bool remove_stale_socket(const std::string& path) {
    struct stat buffer{};
    if (::stat(path.c_str(), &buffer) != 0) {
        return true;
    }

    zdotool::Fd probe{::socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0)};
    if (!probe.valid()) {
        std::println(stderr, "{}: failed to create probe socket: {}", zdotool::daemon_program,
                     std::strerror(errno));
        return false;
    }

    auto address = make_address(path);
    if (::connect(probe.get(), reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == 0) {
        std::println(stderr, "{}: another daemon is already running with socket '{}'",
                     zdotool::daemon_program, path);
        return false;
    }

    std::println("Removing stale socket '{}'", path);
    if (::unlink(path.c_str()) != 0) {
        std::println(stderr, "{}: failed to remove stale socket: {}", zdotool::daemon_program,
                     std::strerror(errno));
        return false;
    }

    return true;
}

bool parse_ownership(const std::string& text, uid_t& uid, gid_t& gid) {
    const auto colon = text.find(':');
    if (colon == std::string::npos) {
        return false;
    }

    const auto parsed_uid = zdotool::parse_int(std::string_view{text}.substr(0, colon));
    const auto parsed_gid = zdotool::parse_int(std::string_view{text}.substr(colon + 1));
    if (!parsed_uid || !parsed_gid || *parsed_uid < 0 || *parsed_gid < 0) {
        return false;
    }

    uid = static_cast<uid_t>(*parsed_uid);
    gid = static_cast<gid_t>(*parsed_gid);
    return true;
}

void disable_pointer_acceleration() {
    if (std::getenv("DISPLAY") == nullptr) {
        return;
    }

    constexpr const char* xinput_path = "/usr/bin/xinput";
    if (::access(xinput_path, X_OK) != 0) {
        std::println(stderr, "{}: '{}' not found; not disabling pointer acceleration",
                     zdotool::daemon_program, xinput_path);
        return;
    }

    const pid_t child = ::fork();
    if (child == 0) {
        ::execl(xinput_path, "xinput", "--set-prop", "pointer:ydotoold virtual device",
                "libinput Accel Profile Enabled", "0,", "1", static_cast<char*>(nullptr));
        ::_exit(2);
    }
}

} // namespace

int zdotool::cmd_daemon(int argc, char** argv) {
    Args args{argc, argv, specs};

    if (!args.parse()) {
        std::println(stderr, "{}: {}", zdotool::daemon_program, args.error());
        return 1;
    }

    if (args.has("-h", "--help")) {
        std::print("{}", zdotool::help::daemon);
        return 0;
    }

    if (args.has("-V", "--version")) {
        std::println("{} {}", zdotool::daemon_program, zdotool::version);
        return 0;
    }

    if (!args.positional().empty()) {
        std::println(stderr, "{}: unexpected argument '{}'", zdotool::daemon_program,
                     args.positional().front());
        return 1;
    }

    DaemonOptions options;
    if (const auto value = args.value("-p", "--socket-path")) {
        options.socket_path = std::string{*value};
    }
    if (const auto value = args.value("-P", "--socket-perm")) {
        options.socket_perm = std::string{*value};
    }
    if (const auto value = args.value("-o", "--socket-own")) {
        options.socket_own = std::string{*value};
    }

    options.setup.mouse = !args.has("-m", "--mouse-off");
    options.setup.keyboard = !args.has("-k", "--keyboard-off");
    options.setup.touch = args.has("-T", "--touch-on");

    if (!args.ok()) {
        std::println(stderr, "{}: {}", zdotool::daemon_program, args.error());
        return 1;
    }

    if (options.socket_path.size() >= socket_path_max) {
        std::println(stderr, "{}: socket path is too long (max {})", zdotool::daemon_program,
                     socket_path_max - 1);
        return 2;
    }

    char* end = nullptr;
    const long parsed_mode = std::strtol(options.socket_perm.c_str(), &end, 8);
    if (end == options.socket_perm.c_str() || *end != '\0' || parsed_mode < 0 ||
        parsed_mode > 0777) {
        std::println(stderr, "{}: invalid socket permission '{}'", zdotool::daemon_program,
                     options.socket_perm);
        return 2;
    }
    const auto socket_mode = static_cast<mode_t>(parsed_mode);

    std::optional<uid_t> owner_uid;
    std::optional<gid_t> owner_gid;
    if (!options.socket_own.empty()) {
        uid_t uid = 0;
        gid_t gid = 0;
        if (!parse_ownership(options.socket_own, uid, gid)) {
            std::println(stderr, "{}: invalid ownership '{}', expected UID:GID",
                         zdotool::daemon_program, options.socket_own);
            return 2;
        }
        owner_uid = uid;
        owner_gid = gid;
    }

    if (::getuid() != 0 || ::getegid() != 0) {
        std::println(stderr, "You are advised to run {} as root, or YMMV.", zdotool::daemon_program);
    }

    zdotool::Fd uinput{::open("/dev/uinput", O_WRONLY | O_CLOEXEC)};
    if (!uinput.valid()) {
        std::println(stderr, "{}: failed to open /dev/uinput: {}", zdotool::daemon_program,
                     std::strerror(errno));
        return 2;
    }

    std::println("Socket path: {}", options.socket_path);

    if (!remove_stale_socket(options.socket_path)) {
        return 2;
    }

    zdotool::Fd server{::socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0)};
    if (!server.valid()) {
        std::println(stderr, "{}: failed to create socket: {}", zdotool::daemon_program,
                     std::strerror(errno));
        return 2;
    }

    auto address = make_address(options.socket_path);
    if (::bind(server.get(), reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0) {
        std::println(stderr, "{}: failed to bind '{}': {}", zdotool::daemon_program,
                     options.socket_path, std::strerror(errno));
        return 2;
    }

    if (::chmod(options.socket_path.c_str(), socket_mode) != 0) {
        std::println(stderr, "{}: failed to change socket permission: {}", zdotool::daemon_program,
                     std::strerror(errno));
        return 2;
    }

    std::println("Socket permission: {}", options.socket_perm);

    if (owner_uid && owner_gid) {
        if (::chown(options.socket_path.c_str(), *owner_uid, *owner_gid) != 0) {
            std::println(stderr, "{}: failed to change socket ownership: {}",
                         zdotool::daemon_program, std::strerror(errno));
            return 2;
        }

        std::println("Socket ownership: UID={}, GID={}", *owner_uid, *owner_gid);
    }

    if (!configure_device(uinput, options.setup)) {
        return 2;
    }

    zdotool::sleep_ms(1000);
    disable_pointer_acceleration();

    std::println("READY");
    std::fflush(stdout);

    input_event event{};
    while (true) {
        const ssize_t received = ::recv(server.get(), &event, sizeof(event), 0);
        if (received == static_cast<ssize_t>(sizeof(event))) {
            (void)::write(uinput.get(), &event, sizeof(event));
        } else if (received < 0 && errno == EINTR) {
            continue;
        }
    }
}
