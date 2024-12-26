#ifndef SERVER_TCP_H
#define SERVER_TCP_H

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <dirent.h>
#include <ctype.h>
#include <signal.h>

#include "constants.h"


/***
 * @brief Handles the TCP protocol for incoming client connections.
 * 
 * @param port string with the port number to listen for TCP connections.
 * @param verbose integer flag for enabling verbose logging of actions and client information.
 */
void tcp_protocol (char* port, int verbose);

/***
 * @brief Sets up and opens a TCP socket for listening to incoming connections.
 * 
 * @param port string representing the port number on which the socket will listen.
 */
void TCP_OpenSocket(char *port);

/***
 * @brief Sends data to a TCP connection in chunks until all bytes are transmitted.
 * 
 * @param ptr pointer to the buffer containing the data to be sent.
 * @param to_write integer indicating the total number of bytes to be sent.
 * @param verbose integer flag indicating whether verbose output is enabled.
 * 
 * @return 1 on successful transmission of all bytes, or 0 if an error occurred.
 */
int writeToTCP(char *ptr, int to_write, int verbose);


/***
 * @brief Reads and sends a file to a TCP connection, adhering to a size limit.
 * 
 * @param filename string containing the name of the file to be sent.
 * @param folder string containing the folder path where the file is located.
 * @param buffer_TCP pointer to the buffer used for reading file content and sending data.
 * @param verbose integer flag indicating whether verbose output is enabled.
 * 
 * @return 1 on successful file transmission, or 0 if an error occurred.
 */
int writeToTCP(char *ptr, int to_write, int verbose);

/***
 * @brief Sends a file over a TCP connection, ensuring compliance with protocol limits.
 * 
 * @param filename Pointer to a string containing the name of the file to send.
 * @param folder Pointer to a string containing the path to the folder where the file is located.
 * @param buffer_TCP Pointer to a buffer used for reading file content and sending data.
 * @param verbose Integer flag indicating if detailed logs should be printed (1 for enabled, 0 for disabled).
 * 
 * @return Returns 1 on successful transmission, or 0 on failure.
 */
int writeFile(char *filename, char *folder, char *buffer_TCP, int verbose);

/***
 * @brief Checks if a given string contains only numeric characters.
 * 
 * @param str Pointer to the string to be checked.
 * 
 * @return Returns 1 if the string is numeric, or 0 otherwise.
 */
int isNumericTCP(char *str);

/***
 * @brief Reads a Player ID (PLID) from a TCP connection and validates its format.
 * 
 * @param PLID Pointer to a buffer where the PLID will be stored.
 * @param verbose If set to 1, prints the received PLID for debugging purposes.
 * 
 * @return Returns 1 if the PLID is valid and successfully read, or 0 if invalid.
 */
int readPLID(char *PLID, int verbose);

/***
 * @brief Finds and compiles the top 10 scores from the scores directory into a formatted string.
 * 
 * @param sb_file Buffer to store the formatted scoreboard as a string.
 * 
 * @return The number of scores found and added to the scoreboard (up to 10). Returns 0 if no scores are found.
 */
int findTopScores(char sb_file[MAX_FILE_SIZE + 1]);

/***
 * @brief Processes the scoreboard request and sends the top 10 scores to the client.
 * 
 * @param verbose If set to 1, enables debug messages for the sent responses.
 */
void scoreboard(int verbose);

/***
 * @brief Finds the most recent game file for a given player.
 * 
 * @param PLID The player ID whose games are to be searched.
 * @param filename The output parameter to store the full path of the most recent game file.
 * 
 * @return Returns 1 if a file is found, otherwise returns 0.
 */
int findLastGame(char *PLID, char *filename);

/***
 * @brief Creates a formatted trial file for a given player.
 * 
 * @param PLID The player ID whose game file is being processed.
 * @param filename The full path to the game file to be read.
 * @param file The output buffer to store the formatted trial summary.
 * @param status Indicates whether the game is active (1) or finalized (0).
 */
void createTrialFile(char *PLID, char *filename, char file[MAX_FILE_SIZE + 1], int status);

/***
 * @brief Handles the "show trials" request from the client.
 * 
 * @param verbose Enables detailed logging if set to a non-zero value.
 */
void show_trials(int verbose);



#endif