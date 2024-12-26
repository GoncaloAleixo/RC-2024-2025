#include <signal.h>
#include "constants.h"
#include "auxiliar_player.h"

int errno;
char* port = "58064"; // Default port for group 64
char hostname[MAX_HOSTNAME_SIZE + 1] = "\0";

char buffer[MAX_READ_SIZE + 1], PLID[MAX_PLID_SIZE + 1];
int trial_number = 0;

void INThandler(int sig);

int main(int argc, char** argv) {
    // Read hostname and port from command-line arguments
    for (int i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "-n")) {
            strcpy(hostname, argv[i + 1]);
        } else if (!strcmp(argv[i], "-p")) {
            port = argv[i + 1];
        }
    }

    // Default hostname: local machine
    if (strlen(hostname) == 0) {
        if (gethostname(buffer, MAX_READ_SIZE) == -1) {
            fprintf(stderr, "error: %s\n", strerror(errno));
            exit(1);
        } else {
            strcpy(hostname, buffer);
        }
    }

    // Validate port
    if (atoi(port) < 1024 || atoi(port) > 65535) {
        fprintf(stderr, "error: invalid port number\n");
        exit(1);
    }

    // Set up signal handler
    signal(SIGINT, INThandler);

    PLID[0] = '\0';
    printf("Welcome to Master Mind Game :D\n");

    char command[10];
    fd_set readfds;
    int counter;
    int max_playtime;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        counter = select(1, &readfds, NULL, NULL, NULL);
        if (counter == -1) {
            fprintf(stderr, "error: %s\n", strerror(errno));
            exit(1);
        }

        if (FD_ISSET(0, &readfds)) {
            // Read command
            if (scanf("%s", command) != 1) {
                while (getchar() != '\n'); // Clear input on invalid read
                continue;
            }

            if (!strcmp(command, "start")) {
                    start(hostname, port, buffer, PLID, &trial_number, &max_playtime);
            } else if (!strcmp(command, "try")) {
                if (trial_number > 0)
                    try(hostname, port, buffer, PLID, &trial_number);
                else
                    printf(NO_GAME);
            } else if (!strcmp(command, "debug")) {
                    debug(hostname, port, buffer, PLID, &trial_number, &max_playtime);
            } else if(!strcmp(command, "scoreboard") || !strcmp(command, "sb")){
                    scoreboard(hostname, port, buffer);
            } else if(!strcmp(command, "show_trials") || !strcmp(command, "st")){
                if(PLID[0]!='\0')
                    show_trials(hostname, port, buffer, PLID, &trial_number);
                else
                    printf(NO_STATE);
            }else if (!strcmp(command, "quit")) {
                if (trial_number > 0)
                    quit(hostname, port, buffer, PLID, &trial_number);
                else
                    printf(NO_GAME);
            } else if (!strcmp(command, "exit")) {
                if (trial_number > 0)
                    quit(hostname, port, buffer, PLID, &trial_number);
                printf(EXIT);
                break;
            } else {
                printf(INVALID_COMMAND);
                while (getchar() != '\n'); // Clear buffer after invalid input
            }
        }
    }
    return 0;
}

void INThandler(int sig) {
    signal(sig, SIG_IGN);
    printf("\n");
    if (trial_number > 0)
        quit(hostname, port, buffer, PLID, &trial_number);
    else
        printf(NO_GAME);
    printf(EXIT);
    exit(0);
}
