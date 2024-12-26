#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAX_READ_SIZE 128
#define MAX_FILENAME_SIZE 24
#define SCORE_FILENAME_SIZE 40
#define COMMAND_SIZE 3
#define PLID_SIZE 6
#define TIME_SIZE 3
#define SECRET_KEY_SIZE 4       // CCCC
#define OUTPUT_KEY_SIZE 7       // C C C C
#define SIZE_DATE 30        // YYYY−MM−DD HH:MM:SS s(s=10 caracters)
#define MAX_WORD_LENGTH 30
#define MAX_TRIALS_SIZE 1
#define MAX_TRIALS 8
#define MAX_TIME 600
#define MAX_FILE_SIZE 1024^3
#define MAX_PLID_SIZE 6

#define FOLDER_GAMES "GS/GAMES/"
#define FOLDER_SCORES "GS/SCORES/"

#define RECEIVE_FAILED "Failed to receive message from server\n"
#define SEND_FAILED "Failed to send message to server\n"

#define BYE_SERVER "Server is shutting down\n"

#define ADDRESS_USED "The address is already in use\n"

#endif