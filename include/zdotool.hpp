#pragma once

#include "args.hpp"
#include "client.hpp"
#include "common.hpp"
#include "help.hpp"
#include "keys.hpp"

namespace zdotool {

using CommandFn = int (*)(Client& client, int argc, char** argv);

int cmd_click(Client& client, int argc, char** argv);
int cmd_key(Client& client, int argc, char** argv);
int cmd_mousemove(Client& client, int argc, char** argv);
int cmd_stdin(Client& client, int argc, char** argv);
int cmd_type(Client& client, int argc, char** argv);
int cmd_daemon(int argc, char** argv);

} // namespace zdotool
