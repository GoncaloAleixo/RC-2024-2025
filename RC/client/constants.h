#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAX_FILENAME_SIZE 40
#define MAX_FSIZE_SIZE 10
#define MAX_READ_SIZE 128
#define MAX_PLID_SIZE 6
#define MAX_COMMAND_SIZE 3
#define MAX_PLAYER_STATE_SIZE 37
#define MAX_HOSTNAME_SIZE 253
#define TIMEOUT 20

#define SCOREBOARD 0
#define SHOW_TRIALS 1

#define INVALID_COMMAND "Invalid command\n"
#define INVALID_TRIAL "Invalid trial number\n"
#define INVALID_KEY "Invalid key!\n"
#define INVALID_INPUT_START "Invalid input! Provide valid PLID (6 digits) and time (1-600).\n"
#define INVALID_INPUT_DEBUG "Invalid input! Provide valid PLID (6 digits), time (1-600), and exactly 4 valid colors.\n"

#define FORMAT_ERROR "Format error\n"
#define ERROR "Error\n"

#define NEW_GAME "New game started (max %d sec): \n"

#define FEEDBACK "Feedback: nB = %d, nW = %d . Trial number: %d\n"
#define DUP_TRY "Duplicate try! This key has already been submitted.\n"
#define WIN_MESSAGE "WELL DONE! You guessed the key in %d trial(s)!\n"
#define GAME_OVER "Game over! The secret key was: %s\n"
#define TIME_OVER "Time's up! The secret key was: %s\n"
#define GAME_ONGOING "ERROR: A game is already ongoing for this player.\n"

#define NO_SCORES "No game was yet won by any player\n"
#define NO_STATE "No games active or finished for you :/\n"

#define QUIT "Game terminated. Secret key: %s\n"
#define EXIT "Application terminated. Bye!\n"
#define NO_GAME "There is no game in progress\n"

#define RECEIVED_SHOW_TRIALS "Trials file received: %s | Size: %zu bytes\n"
#define RECEIVED_SCOREBOARD "Scoreboard file received: %s | Size: %zu bytes\n"

#define SEND_FAILED "Failed to send message to server\n"
#define RECEIVE_FAILED "Failed to receive message from server\n"

#define DEBUG_STARTED "Debug game started (time = %d sec) with key: %c %c %c %c\n"
#define DEBUG_ONGOING "Cannot start debug mode: another game is in progress.\n"



#endif
