#ifndef AUXILIAR_PLAYER_H
#define AUXILIAR_PLAYER_H

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

/***
 * @brief sets up a UDP (User Datagram Protocol) socket
 * 
 * @param hostname string with the hostname
 * @param port string with the ip
*/
void createUDPsocket(char* hostname, char* port);

/***
 * @brief sets up a TCP (Transmission Control Protocol) socket
 * 
 * @param hostname string with the hostname
 * @param port string with the ip
*/
void createTCPsocket(char* hostname, char* port);

//Releases resources associated with the addrinfo structure and closes the socket
void closeSocket();

/***
 * @brief Sends message to Server, and waits response
 *        Closes socket after communication is complete
 * 
 * @param buffer character array to hold data
 * @param hostname string with the hostname
 * @param port string with the ip
*/
int sendAndReadUDP(char *buffer, char *hostname, char *port);

/***
 * @brief Sends message to Server, and waits response
 *        Closes socket after communication is complete
 * 
 * @param buffer character array to hold data
*/
int sendAndReadTCP(char *buffer);

/***
 * @brief Reads a specified amount of data
 *        from the TCP socket into a buffer
 *        Ensures all requested data is read or exits on error
 * 
 * @param buffer character array to hold data
 * @param nleft size of message
*/
ssize_t readFromTCPsocket(char *buffer, ssize_t nleft);

/***
 * @brief Reads a file transmitted from the server over a TCP connection. 
 *        It processes metadata, retrieves the file's contents,
 *        and optionally prints it depending on the command 
 * 
 * @param command int with the type of file (0/1)
 * @param buffer character array to hold data 
*/
void readFile(int command, char *buffer);

/***
 * @brief Initiates a new Master Mind game.
 *
 * @param hostname The hostname or IP address of the server.
 * @param port The port number to connect to the server.
 * @param buffer A buffer to hold messages to and from the server.
 * @param PLID The player's unique ID (6-digit number).
 * @param trial_number Pointer to the current trial number (reset on success).
 * @param max_playtime Pointer to the maximum playtime allowed for the game.
*/
void start(char* hostname, char* port, char *buffer, char *PLID, int *trial_number, int *max_playtime);

/***
 * @brief Sends a trial guess to the server and processes the response.
 *
 * @param hostname The hostname or IP address of the server.
 * @param port The port number to connect to the server.
 * @param buffer A buffer to hold messages to and from the server.
 * @param PLID The player's unique ID (6-digit number).
 * @param trial_number Pointer to the current trial number.
*/
void try(char* hostname, char* port, char *buffer, char *PLID, int *trial_number);

/**
 * @brief Starts a debug game session with the server.
 *
 * @param hostname The hostname or IP address of the server.
 * @param port The port number to connect to the server.
 * @param buffer A buffer to hold messages to and from the server.
 * @param PLID The player's unique ID (6-digit number).
 * @param trial_number Pointer to the current trial number (reset to 1 on success).
 * @param max_playtime Pointer to store the maximum game time allowed (in seconds).
*/
void debug(char* hostname, char* port, char* buffer, char* PLID, int* trial_number, int* max_playtime);

/***
 * @brief Requests and displays the top-10 scoreboard from the Game Server (GS).
 *
 * @param hostname The hostname or IP address of the server.
 * @param port The port number to connect to the server.
 * @param buffer A buffer to hold messages to and from the server.
*/
void scoreboard(char* hostname, char* port, char *buffer);

/**
 * @brief Displays the list of trials and corresponding results for the current or most recent game.
 *
 * @param hostname The hostname or IP address of the server.
 * @param port The port number to connect to the server.
 * @param buffer A buffer to hold messages to and from the server.
 * @param PLID The player's unique ID (6-digit number).
 * @param trial_number Pointer to the current trial number (reset if the game is finished).
*/
void show_trials(char* hostname, char* port, char *buffer, char *PLID, int *trial_number);

/***
 * @brief Terminates an ongoing Master Mind game.
 *
 * @param hostname The hostname or IP address of the server.
 * @param port The port number to connect to the server.
 * @param buffer A buffer to hold messages to and from the server.
 * @param PLID The player's unique ID (6-digit number).
 * @param trial_number Pointer to the current trial number (reset on success).
*/
void quit(char* hostname, char* port, char *buffer, char *PLID, int *trial_number);

// Checks if input is a valid Key combination
int validKey(char C1, char C2, char C3, char C4);

// Helper function to validate PLID
int validPLID(char *PLID);

// Clear input buffer
void clearInputBuffer();

// Reads data one character at a time until a delimiter is encountered
int readUntilSpace(char *ptr);

//checks whether a given string consists entirely of numeric characters
int isNumeric(char *str);

#endif
