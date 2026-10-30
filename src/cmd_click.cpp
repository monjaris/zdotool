#include "zdotool.hpp"
#include "help.hpp"


static void show_help() {
    puts(
        help::click
    );
}

int tool_click(int argc, char** argv) {
    if (argc < 2) {
        show_help();
        return 0;
    }

    int repeats = 1;
    int next_delay_ms = 25;
    bool press_release = 0;

    while (1) {
        int c;

        static struct option long_options[] = {
            {"repeat", required_argument, 0, 'r'},
            {"next-delay", required_argument, 0, 'D'},
            {"press-release", no_argument, 0, 'P'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "hPr:D:",
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
            case 'r':
                repeats = strtol(optarg, NULL, 10);
                break;

            case 'D':
                next_delay_ms = strtol(optarg, NULL, 10);
                break;
            
            case 'P':
                press_release = 1;
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
        int optind_save = optind;

        for (int i=0; i<repeats; i++) {
            optind = optind_save;
            while (optind < argc) {
                int key = strtol(argv[optind++], NULL, 16);
                int keycode = (key & 0xf) | 0x110;

                if (press_release) {
                    printf("Click Event %d\n", i);
                    uinput_emit(EV_KEY, keycode, 1, 1);
                    uinput_emit(EV_KEY, keycode, 0, 1);
                    usleep(next_delay_ms * 1000);
                    continue;
                }

                if (key & 0x40) {
                    uinput_emit(EV_KEY, keycode, 1, 1);
                    usleep(next_delay_ms * 1000);
                }

                if (key & 0x80) {
                    uinput_emit(EV_KEY, keycode, 0, 1);
                    usleep(next_delay_ms * 1000);
                }

                if ((key & 0xc0) == 0) {
                    usleep(next_delay_ms * 1000);
                }

                printf("%x %x\n", key, keycode);
            }
        }
    } else {
        show_help();
    }

    return 0;
}
