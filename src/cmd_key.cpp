#include "zdotool.hpp"
#include "help.hpp"

static void show_help() {
    puts(
        help::key
    );
}


int tool_key(int argc, char** argv) {
    if (argc < 2) {
        show_help();
        return 0;
    }

    int key_delay = 12;

    while (1) {
        int c;

        static struct option long_options[] = {
            {"key-delay", required_argument, 0, 'd'},
            {"file", required_argument, 0, 'f'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "hd:",
                 long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;
            case 'D':
                key_delay = strtol(optarg, NULL, 10);
                break;

            case 'd':
                key_delay = strtol(optarg, NULL, 10);
                break;

            case 'h':
                show_help();
                exit(0);
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                abort();
        }
    }


    if (optind < argc) {
        while (optind < argc) {
            char* pstr = argv[optind++];
            uint16_t kc;
            if (strchr(pstr, 'x')) {
                kc = strtol(pstr, NULL, 16);
            } else {
                kc = strtol(pstr, NULL, 10);
            }

            size_t slen = strlen(pstr);
            if (slen) {
                char cen = pstr[slen-1];

                if (cen == '0') {
                    uinput_emit(EV_KEY, kc, 0, 1);
                } else {
                    uinput_emit(EV_KEY, kc, 1, 1);
                }
            }

            usleep(key_delay * 1000);
        }
    } else {
        show_help();
    }

    return 0;
}
