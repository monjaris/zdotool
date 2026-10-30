#pragma once
//
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cassert>
#include <ctime>
#include <csignal>
//
#include <getopt.h>
//



#if defined(__unix__)
#   include <unistd.h>
#   include <fcntl.h>
#   include <dirent.h>
#   include <dlfcn.h>
#   include <sys/socket.h>
#   include <sys/stat.h>
#   include <sys/ioctl.h>
#   include <sys/mman.h>
#   include <sys/types.h>
#   include <sys/un.h>
#   include <sys/epoll.h>
#else
#   error Use linux...
#endif


#if defined(__unix__)
#   include <linux/uinput.h>
#endif


extern void uinput_emit(uint16_t type, uint16_t code, int32_t val, bool syn_report);
extern int tool_click(int argc, char** argv);
extern int tool_mousemove(int argc, char** argv);
extern int tool_type(int argc, char** argv);
extern int tool_key(int argc, char** argv);
extern int tool_stdin(int argc, char** argv);
