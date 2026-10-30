#include "zdotool.hpp"
#include "help.hpp"

static void show_help() {
    puts(
        help::mousemove
    );
}

int tool_mousemove(int argc, char** argv) {
    if (argc < 2) {
        show_help();
        return 0;
    }

    int is_abs = 0;
    int is_wheel = 0;

    int i = 0;
    int32_t pos[2] = {0, 0};

    while (1) {
        int c;

        static struct option long_options[] = {
            {"absolute", no_argument, 0, 'a'},
            {"help", no_argument, 0, 'h'},
            {"wheel", no_argument, 0, 'w'},
            {"xpos", required_argument, 0, 'x'},
            {"ypos", required_argument, 0, 'y'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "hawx:y:",
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
            case 'a':
                is_abs = 1;
                break;

            case 'h':
                show_help();
                return 0;

            case 'w':
                is_wheel = 1;
                break;
            case 'x':
                if (i >= 2) {
                    show_help();
                    return 1;
                }
                pos[0] = strtol(optarg, NULL, 10);
                i++;
                break;
            case 'y':
                if (i >= 2) {
                    show_help();
                    return 1;
                }
                pos[1] = strtol(optarg, NULL, 10);
                i++;
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
            if (i >= 2) {
                show_help();
                return 1;
            }
            pos[i] = strtol(argv[optind++], NULL, 10);
            i++;
        }
    }

    if (i == 2) {
        if (is_abs && is_wheel) {
            puts("Mouse wheel moves should not be absolute values");
            return 1;
        }

        if (is_abs) {
            uinput_emit(EV_REL, REL_X, INT32_MIN, 0);
            uinput_emit(EV_REL, REL_Y, INT32_MIN, 1);
        }

        if (is_wheel) {
            uinput_emit(EV_REL, REL_HWHEEL, pos[0], 0);
            uinput_emit(EV_REL, REL_WHEEL, pos[1], 1);
        } else {
            uinput_emit(EV_REL, REL_X, pos[0], 0);
            uinput_emit(EV_REL, REL_Y, pos[1], 1);
        }
    } else {
        show_help();
        return 1;
    }

    return 0;
}
