# zdotool

A small C++23 Linux command-line automation tool that injects input through
`uinput`, with no X11 or Wayland dependency.

`zdotool` talks to a long-running `zdotool daemon` over a Unix datagram socket.
The socket path, wire format, and virtual-device name are all zdotool-specific.

## Commands

- `click` — click mouse buttons
- `daemon` — run the input daemon
- `key` — emit raw keycodes
- `mousemove` — move the pointer or the wheel
- `stdin` — forward terminal input as key events
- `type` — type strings

Run `zdotool <command> --help` for the options of each command.

## Build

Requirements:

- Linux
- `xmake`
- A C++23 compiler and standard library with `<print>` support

```sh
xmake
```

xmake fetches and statically links [clixx](https://github.com/monjaris/clixx)
from the `mj-xrepo` repository. The resulting `zdotool` binary is copied to the
repository root.

## Usage

Start the daemon:

```sh
sudo zdotool daemon
```

The daemon needs access to `/dev/uinput`. On most distributions you can instead
add your user to the `input` group and run `zdotool daemon` without root:

```sh
sudo usermod -aG input "$USER"
```

Then inject input:

```sh
zdotool mousemove -x 100 -y 100
zdotool click 0xC0
zdotool type 'hello'
zdotool key 29:1 56:1 59:1 59:0 56:0 29:0
```

The daemon chooses its socket path in this order:

1. `-p PATH` on the command line
2. `$ZDOTOOL_SOCKET`
3. `$XDG_RUNTIME_DIR/.zdotool_socket`
4. `/tmp/.zdotool_socket`

The client and daemon use the same resolution, so they normally find each other
without extra configuration.

### Services

Example service files are in `packaging/`:

- `packaging/systemd/zdotool-daemon.service`
- `packaging/openrc/zdotool-daemon`

They assume `zdotool` is installed in `/usr/local/bin`.

## License

AGPL-3.0. See [LICENSE.txt](LICENSE.txt).

This project is derived from ydotool by ReimuNotMoe and contributors.
