#include "zdotool.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>


struct tool_def {
	char name[16];
	int (*ptr)(int, char**);
};

int fd_daemon_socket = -1;

static int tool_debug(int argc, char** argv) {
	printf("fd_daemon_socket: %d\n", fd_daemon_socket);
	printf("argc: %d\n", argc);

	for (int i=0; i<argc; i++) {
		printf("argv[%d]: %s\n", i, argv[i]);
	}

	return 0;
}

static int tool_bakers(int argc, char** argv) {
	puts("These are our honorable bakers:\n"
	     "\n"
	     "Dustin Van Tate Testa\n"
	     "Elliot Wolk\n"
	     "tofik\n"
	);

	return 0;
}

static const struct tool_def tool_list[] = {
    {"click",     reinterpret_cast<int(*)(int, char**)>(tool_click)},
    {"mousemove", reinterpret_cast<int(*)(int, char**)>(tool_mousemove)},
    {"type",      reinterpret_cast<int(*)(int, char**)>(tool_type)},
    {"key",       reinterpret_cast<int(*)(int, char**)>(tool_key)},
    {"debug",     reinterpret_cast<int(*)(int, char**)>(tool_debug)},
    {"bakers",    reinterpret_cast<int(*)(int, char**)>(tool_bakers)},
    {"stdin",     reinterpret_cast<int(*)(int, char**)>(tool_stdin)},
};

static void show_help() {
	puts("Usage: ydotool <cmd> <args>\n"
	     "Available commands:");

	int tool_count = sizeof(tool_list) / sizeof(struct tool_def);

	for (int i=0; i<tool_count; i++) {
		printf("  %s\n", tool_list[i].name);
	}

	puts("Use environment variable YDOTOOL_SOCKET to specify daemon socket.");
}

void uinput_emit(uint16_t type, uint16_t code, int32_t val, bool syn_report) {
	struct input_event ie = {
		.type = type,
		.code = code,
		.value = val
	};

	write(fd_daemon_socket, &ie, sizeof(ie));

	if (syn_report) {
		ie.type = EV_SYN;
		ie.code = SYN_REPORT;
		ie.value = 0;
		write(fd_daemon_socket, &ie, sizeof(ie));
	}

}

int main(int argc, char** argv) {
	if (argc < 2 || strncmp(argv[1], "-h", 2) == 0 || strncmp(argv[1], "--h", 3) == 0 || strcmp(argv[1], "help") == 0) {
		show_help();
		return 0;
	}

	int (*tool_main)(int argc, char** argv) = NULL;

	int tool_count = sizeof(tool_list) / sizeof(struct tool_def);

	for (int i=0; i<tool_count; i++) {
		if (strcmp(tool_list[i].name, argv[1]) == 0) {
			tool_main = reinterpret_cast<int(*)(int, char**)>(tool_list[i].ptr);
		}
	}

	if (!tool_main) {
		printf("ydotool: Unknown command: %s\n"
		       "Run 'ydotool help' if you want a command list\n", argv[1]);
		return 1;
	}

	fd_daemon_socket = socket(AF_UNIX, SOCK_DGRAM, 0);

	if (fd_daemon_socket < 0) {
		perror("failed to create socket");
		exit(2);
	}

	struct sockaddr_un sa = {
		.sun_family = AF_UNIX
	};

	char* env_ys = getenv("YDOTOOL_SOCKET");
	char* env_xrd = getenv("XDG_RUNTIME_DIR");

	if (env_ys) {
		snprintf(sa.sun_path, sizeof(sa.sun_path)-1, "%s", env_ys);
	} else if (env_xrd){
		snprintf(sa.sun_path, sizeof(sa.sun_path)-1, "%s/.ydotool_socket", env_xrd);
	} else {
		snprintf(sa.sun_path, sizeof(sa.sun_path)-1, "%s", "/tmp/.ydotool_socket");
	}

	if (connect(fd_daemon_socket, (const struct sockaddr *) &sa, sizeof(sa))) {
		int err = errno;
		printf("failed to connect socket `%s': %s\n", sa.sun_path, strerror(err));

		switch (err) {
			case ENOENT:
			case ECONNREFUSED:
				puts("Please check if ydotoold is running.");
				break;
			case EACCES:
			case EPERM:
				puts("Please check if the current user has sufficient permissions to access the socket file.");
				break;
		}

		exit(2);
	}

	return tool_main(argc-1, argv+1);
}
