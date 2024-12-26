#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>

#include "constants.h"

/***
 * @brief Handles the UDP protocol for incoming client requests.
 * 
 * @param port string with the port number to listen for UDP connections.
 * @param verbose integer flag for enabling verbose logging of actions and client information.
 */
void udp_protocol (char* port, int verbose);

/***
 * @brief Establishes a UDP socket, binds it to the specified port, and prepares it to receive data.
 * 
 * @param port string with the port number to bind the UDP socket.
 */
void udp_connect(char *port);

//Sends a UDP message stored in the global buffer to the client.
void sendtoUDP();

/***
 * @brief Validates the format of a Player ID (PLID).
 * 
 * @param plid string representing the Player ID to validate.
 * @return 1 if the PLID is valid, 0 otherwise.
 */
int validPlid(char *plid);

/***
 * @brief Validates the format of a time string.
 * 
 * @param time string representing the time to validate.
 * @return 1 if the time is valid, 0 otherwise.
 */
int validTime(char *time);

/***
 * @brief Generates a random secret key for the Mastermind game.
 * 
 * @param secretKey pointer to a character array where the generated secret key will be stored. 
 *                  The array must have enough space to hold 5 characters (4 for the key + 1 for null-termination).
 */ 
void generateSecretKey(char *secretKey);

/***
 * @brief Creates a game file to store game details for a player.
 * 
 * @param filename string representing the name of the game file to create.
 * @param plid string containing the Player ID associated with the game.
 * @param mode character representing the game mode (e.g., 'P' for PLAY, 'D' for DEBUG).
 * @param secretKey string containing the secret key for the game.
 * @param maxTime string representing the maximum allowed playtime in seconds.
 * 
 * @note If the file cannot be created, the function prints an error and terminates the program.
 */
void createGameFile(char *filename, char *plid, char mode, char *secretKey, char *maxTime);

/***
 * @brief Retrieves the game mode from a given game file.
 * 
 * @param filename string representing the name of the game file to read.
 * @return character representing the game mode.
 */
char getGameMode(const char *filename);

/***
 * @brief Retrieves the secret key from a given game file.
 * 
 * @param filename string representing the name of the game file to read.
 * @param secretKey pointer to a character array where the retrieved secret key will 
 *                  be stored. The array must be large enough to hold the secret key.
 */
void getSecretKey (const char *filename, char *secretKey);

/***
 * @brief Retrieves the start time of a game from a GAME file.
 *
 * @param filename string representing the name of the GAME file to read.
 * @return integer representing the start time of the game.
 */
int getStartTime(const char *filename);

/***
 * @brief Retrieves the maximum allowed playtime of a game from a GAME file.
 *
 * @param filename string representing the name of the GAME file to read.
 * @return integer representing the maximum allowed playtime for the game.
 */
int getMaxTime(const char *filename);

/***
 * @brief Checks if the maximum allowed playtime for a game has been exceeded.
 * 
 * @param filename string representing the name of the GAME file to read.
 * @return integer value:
 *         - 1 if the elapsed time exceeds the maximum allowed playtime.
 *         - 0 otherwise.
 */
int exceededTime(const char *filename);

/***
 * @brief Determines the number of trials (lines) recorded in a GAME file.
 * 
 * @param filename string representing the name of the GAME file to read.
 * @return integer representing the total number of lines (trials) in the file.
 */
int getExpectedTrial(const char *filename);

/***
 * @brief Retrieves the last key guess and its evaluation (nB, nW) from a GAME file.
 * 
 * @param filename string representing the name of the GAME file to read.
 * @param previousKeyGuess string buffer to store the last key guess.
 * @param nB pointer to an integer to store the number of correct positions.
 * @param nW pointer to an integer to store the number of correct colors in incorrect positions.
 */
void getPreviousKeyGuess(const char *filename, char *previousKeyGuess, int *nB, int *nW);

/***
 * @brief Calculates the duration of a game in seconds.
 * 
 * @param filename string representing the name of the GAME file to read.
 * @return integer representing the elapsed time in seconds since the game started.
 */
int duration(const char *filename);

/***
 * @brief Handles the "start" command for initiating a new game session.
 * 
 * @param verbose integer flag to enable verbose logging of actions and validation steps.
 * 
 * @details
 * - The function validates the format of the PLID and playtime using `validPlid` and `validTime`.
 * - If a game file already exists for the PLID and the time is exceeded, the game is terminated via `finishGame`.
 * - If input validation fails, it returns an error response `RSG ERR`.
 * - If there is already an ongoing game for the PLID, it returns a `RSG NOK` response.
 * - If all checks pass, it generates a new secret key and creates a game file using `generateSecretKey` 
 *   and `createGameFile`, then returns a success response `RSG OK`.
 * 
 * @note Sends the response back to the client using `sendtoUDP`.
 */
void start(int verbose);

/***
 * @brief Formats a secret key guess using four input characters.
 * 
 * @param secretKeyGuess Pointer to a character array to store the formatted secret key guess.
 * @param C1 First character of the key.
 * @param C2 Second character of the key.
 * @param C3 Third character of the key.
 * @param C4 Fourth character of the key.
 */
void secretKey_format(char *secretKeyGuess, char C1, char C2, char C3, char C4);

/***
 * @brief Formats the secret key from a game file into a spaced output.
 * 
 * @param filename Path to the game file containing the secret key.
 * @param keyOutput Pointer to a character array to store the formatted key output.
 */
void keyOutput_format(const char *filename, char *keyOutput);

/***
 * @brief Validates a secret key guess to ensure it matches the expected format.
 * 
 * @param secretKeyGuess Pointer to a character array representing the secret key guess.
 * @return 1 if the key is valid, 0 otherwise.
 */
int validSecretKey(char *secretKeyGuess);

/***
 * @brief Checks if the provided secret key guess matches any previous guesses in the game file.
 * 
 * This function scans the game file for any previously made guesses and compares them
 * to the provided secret key guess. Returns true if a match is found, indicating a repeat guess.
 * 
 * @param filename Path to the game file containing previous guesses.
 * @param nT Current trial number. Function returns 0 if `nT < 2` (no previous guesses to check).
 * @param secretKeyGuess Pointer to a character array representing the current secret key guess.
 * @return 1 if the key guess is a repeat, 0 otherwise.
 */
int repeatSecretKey(char *filename, int nT, char *secretKeyGuess);

/***
 * @brief Finalizes a game by appending end time and duration to the current game file 
 * and renaming it based on the game's completion status.
 * 
 * @param plid Pointer to a string representing the player's unique ID (PLID).
 * @param filename Path to the active game file that will be finalized.
 * @param code Completion code indicating the game's end status:
 *        'W' for Win, 'F' for Fail, 'T' for Timeout, or 'Q' for Quit.
 */
void finishGame(char *plid, char *filename, char code);

/***
 * @brief Evaluates a player's guess against the secret key, calculating exact and partial matches.
 *
 * @param secretKeyGuess Pointer to the player's guessed secret key (string of colors).
 * @param secretKey Pointer to the actual secret key (string of colors).
 * @param nB Pointer to an integer for storing the number of exact matches (black pegs).
 * @param nW Pointer to an integer for storing the number of partial matches (white pegs).
 */
void evaluateGuess(char *secretKeyGuess, char *secretKey, int *nB, int *nW);

/***
 * @brief Records a player's trial into the game file, appending the guess and its results.
 *
 * @param filename Pointer to the path of the game file.
 * @param secretKeyGuess Pointer to the player's guessed secret key (string of colors).
 * @param nB Integer representing the number of exact matches (black pegs).
 * @param nW Integer representing the number of partial matches (white pegs).
 */
void addTrial(char *filename, char *secretKeyGuess, int nB, int nW);

/***
 * @brief Calculates the player's score based on the number of trials and game duration.
 *
 * @param nT Integer representing the number of trials used by the player.
 * @param duration Integer representing the total duration of the game in seconds.
 * 
 * @return An integer score between 001 and 100.
 */
int calculateScore(int nT, int duration);

/***
 * @brief Creates a score file in the SCORES directory with the game's final results.
 *
 * @param score Integer representing the player's score (calculated using `calculateScore`).
 * @param plid Pointer to the player's unique ID (6-character string).
 * @param secretKey Pointer to the secret key guessed by the player (4-character string).
 * @param nT Integer representing the number of trials used by the player.
 * @param mode Character representing the game mode: 'P' for PLAY or 'D' for DEBUG.
 */
void createScoreFile(int score, char *plid, char *secretKey, int nT, char mode);

/***
 * @brief Processes a player's trial in the Mastermind game.
 * 
 * This function handles the evaluation of a player's guess during an ongoing game. 
 * It validates the syntax of the guess, checks game rules (e.g., trial limits, duplicate keys),
 * evaluates the guess against the secret key, and updates the game state accordingly.
 * 
 * @param verbose Integer flag for enabling verbose logging of the game's internal actions and decisions.
 * 
 * @return This function does not return a value. It directly sends the response to the player using `sendtoUDP`.
 */
void try(int verbose);


void quit(int verbose);
void debug(int verbose);
void printVerbose(char *verbose_buffer, char *plid, char *secretKeyGuess, int nT);
#endif