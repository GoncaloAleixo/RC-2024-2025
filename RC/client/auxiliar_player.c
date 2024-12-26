#include "constants.h"
#include "auxiliar_player.h"

int fd, errno, errcode, afd = 0;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;

//-------------------------------- COMMUNICATION PROTOCOLS ----------------------------//

void createUDPsocket(char* hostname, char* port){
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1){
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    errcode = getaddrinfo(hostname, port, &hints, &res);
    if(errcode != 0){
        fprintf(stderr, "error: %s\n", gai_strerror(errcode));
        exit(1);
    }

    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

void createTCPsocket(char* hostname, char* port){
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1){
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    errcode = getaddrinfo(hostname, port, &hints, &res);
    if(errcode != 0){
        fprintf(stderr, "error: %s\n", gai_strerror(errcode));
        exit(1);
    }
    
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

void closeSocket(){
    freeaddrinfo(res);
    close(fd);
}

int sendAndReadUDP(char *buffer, char *hostname, char *port){
    createUDPsocket(hostname, port);

    n = sendto(fd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);
    if(n == -1){
        printf(SEND_FAILED);
        return 0;
    }

    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer, MAX_READ_SIZE, 0, (struct sockaddr*)&addr, &addrlen);
    if(n == -1){
        printf(RECEIVE_FAILED);
        return 0;
    }
    buffer[n]='\0';
    
    closeSocket();
    return 1;
}

int sendAndReadTCP(char *buffer){
    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if(n == -1){
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(1);
    }

    ssize_t nleft = strlen(buffer);
    char *ptr = buffer;
    while(nleft>0){
        n = write(fd, ptr, strlen(buffer));
        if(n == -1){
            printf(SEND_FAILED);
            return 0;
        }
        nleft-=n; ptr+=n;
    }

    return readUntilSpace(buffer);
}

ssize_t readFromTCPsocket(char *buffer, ssize_t nleft){
    char *ptr = buffer;
    ssize_t total_read = 0;
    while(nleft > 0){
        n = read(fd, ptr, nleft);
        if(n == -1){
            fprintf(stderr, "error: %s\n", strerror(errno));
            exit(1);
        } else if (n == 0){
            break;
        }
        nleft -= n;
        ptr += n;
        total_read += n;
    }
    buffer[total_read] = '\0';
    return total_read;
}

void readFile(int command, char *buffer){
    char filename[MAX_FILENAME_SIZE + 1], filesize_str[MAX_FSIZE_SIZE + 1];

    if(!readUntilSpace(filename) || !readUntilSpace(filesize_str))
        return;

    ssize_t filesize = atoi(filesize_str);

    FILE *fp = fopen(filename, "w");
    if(fp == NULL){
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(1);
    }

    ssize_t nleft = filesize;

    while (nleft > 0){
        if(nleft > MAX_READ_SIZE)
            n = readFromTCPsocket(buffer, MAX_READ_SIZE);
        else
            n = readFromTCPsocket(buffer, nleft);
        nleft -= n;
        fwrite(buffer, 1, n, fp);
        if (command == SHOW_TRIALS || command == SCOREBOARD)
            printf("%s", buffer);
    }
    fclose(fp);
    while((n = read(fd, buffer, 1))==0){
        if(n == -1){
            printf(RECEIVE_FAILED);
            exit(1);
        }
    }

    if(command == SHOW_TRIALS)
        printf(RECEIVED_SHOW_TRIALS, filename, filesize);
    else if(command == SCOREBOARD)
        printf(RECEIVED_SCOREBOARD, filename, filesize);
}


//------------------------------------- PLAYER UDP COMMANDS ---------------------------------------------//

void start(char* hostname, char* port, char *buffer, char *PLID, int *trial_number, int *max_playtime) {
    // Input variables
    int time;
    int inputResult;

    // Validate PLID
    inputResult = scanf(" %6s", PLID);
    if (inputResult != 1 || strlen(PLID) != MAX_PLID_SIZE || !isNumeric(PLID)) {
        printf(INVALID_INPUT_START);
        while (getchar() != '\n'); // Clear input buffer
        return;
    }

    // Validate max_playtime
    inputResult = scanf(" %d", &time);
    if (inputResult != 1 || time <= 0 || time > 600) {
        printf(INVALID_INPUT_START);
        while (getchar() != '\n'); // Clear input buffer
        return;
    }

    // Check for trailing input
    char extra;
    if ((extra = getchar()) != '\n' && extra != EOF) {
        printf(INVALID_INPUT_START);
        while (getchar() != '\n'); // Clear input buffer
        return;
    }

    // Store valid time
    *max_playtime = time;

    // Debug print: Confirm parsed input
    printf("Debug: Parsed input: PLID = %s, time = %d\n", PLID, time);

    // Format the SNG command
    sprintf(buffer, "SNG %s %03d\n", PLID, *max_playtime);

    // Send the command and read the response
    if (!sendAndReadUDP(buffer, hostname, port)) {
        return;
    }

    // Parse server response
    char buf1[4], buf2[4], n;
    int i = sscanf(buffer, "%s %s%c", buf1, buf2, &n);

    // Case: RSG OK
    if (i == 3 && !strcmp(buf1, "RSG") && !strcmp(buf2, "OK") && n == '\n') {
        *trial_number = 1; // Reset trial number for new game
        printf(NEW_GAME, *max_playtime);
        return;
    }

    // Case: RSG NOK
    if (i == 3 && !strcmp(buf1, "RSG") && !strcmp(buf2, "NOK") && n == '\n') {
        printf(GAME_ONGOING);
        return;
    }

    // Case: RSG ERR
    if (i == 3 && !strcmp(buf1, "RSG") && !strcmp(buf2, "ERR") && n == '\n') {
        printf(ERROR);
        return;
    }

    // Invalid format
    printf(FORMAT_ERROR);
}

void try(char* hostname, char* port, char *buffer, char *PLID, int *trial_number) {
    char C1, C2, C3, C4;
    int nB = 0, nW = 0, nT;
    char* ptr = buffer; 
    char status[4];
    int inputResult;

    // Read and validate input: exactly 4 colors
    inputResult = scanf(" %c %c %c %c", &C1, &C2, &C3, &C4);
    if (inputResult != 4 || !validKey(C1, C2, C3, C4)) {
        printf(INVALID_KEY);
        while (getchar() != '\n'); // Clear input buffer
        return;
    }

    // Check for trailing input
    char extra;
    if ((extra = getchar()) != '\n' && extra != EOF) {
        printf(INVALID_KEY);
        while (getchar() != '\n'); // Clear input buffer
        return;
    }

    // Prepare and send TRY message
    sprintf(buffer, "TRY %s %c %c %c %c %d\n", PLID, C1, C2, C3, C4, *trial_number);
    if (!sendAndReadUDP(buffer, hostname, port)) {
        return; // Communication error
    }

    printf("Debug: Sent message: '%s'\n", buffer);

    // Parse server response
    sscanf(buffer, "%*s %s", status);
    printf("Debug: Parsed status = '%s'\n", status);

    if (!strcmp(status, "OK")) {
        sscanf(ptr, "%*s %*s %d %d %d", &nT, &nB, &nW);
        if (nT != *trial_number) {
            printf(INVALID_TRIAL);
            return;
        }
        if (nB == 4) { // Win condition
            printf(WIN_MESSAGE, *trial_number);
            *trial_number = 0; // Reset trial number
        } else {
            printf(FEEDBACK, nB, nW, *trial_number);
            (*trial_number)++;
        }
    } else if (!strcmp(status, "DUP")) {
        printf(DUP_TRY);
    } else if (!strcmp(status, "ENT")) {
        char key[16];
        sscanf(ptr, "%*s %*s %15[^\n]", key);
        printf(GAME_OVER, key);
        *trial_number = 0;
    } else if (!strcmp(status, "ETM")) {
        char key[16];
        sscanf(ptr, "%*s %*s %15[^\n]", key);
        printf(TIME_OVER, key);
        *trial_number = 0;
    } else if (!strcmp(status, "NOK")) {
        printf(NO_GAME);
        *trial_number = 0; // Reset game state
    } else if (!strcmp(status, "INV")) {
        printf(INVALID_TRIAL);
    } else {
        printf(FORMAT_ERROR);
    }
}

void debug(char* hostname, char* port, char* buffer, char* PLID, int* trial_number, int* max_playtime) {
    int time;
    char C1, C2, C3, C4;
    int inputResult;

    // Parse and validate input: PLID (6 digits), time (1-600), and exactly 4 colors
    inputResult = scanf("%6s %d %c %c %c %c", PLID, &time, &C1, &C2, &C3, &C4);

    // Check input format and length
    if (inputResult != 6 || strlen(PLID) != 6 || time < 1 || time > 600) {
        printf(INVALID_INPUT_DEBUG);
        while (getchar() != '\n'); // Clear input buffer
        return;
    }

    // Validate colors using validKey function
    if (!validKey(C1, C2, C3, C4)) {
        printf(INVALID_INPUT_DEBUG);
        while (getchar() != '\n'); // Clear input buffer
        return;
    }

    // Check for trailing input
    char extra;
    if ((extra = getchar()) != '\n' && extra != EOF) {
        printf(INVALID_INPUT_DEBUG);
        while (getchar() != '\n'); // Clear input buffer
        return;
    }

    // Debug print: Confirm parsed input
    printf("Debug: Parsed input: PLID = %s, time = %d, colors = %c %c %c %c\n", PLID, time, C1, C2, C3, C4);

    // Prepare the DBG message with PLID
    sprintf(buffer, "DBG %s %d %c %c %c %c\n", PLID, time, C1, C2, C3, C4);

    // Send the message to the server
    if (!sendAndReadUDP(buffer, hostname, port)) {
        return;
    }

    printf("Debug: Sent message: '%s'\n", buffer);

    // Parse server response
    char response[4], status[4];
    if (sscanf(buffer, "%3s %3s", response, status) != 2) {
        printf(RECEIVE_FAILED);
        return;
    }

    printf("Debug: Parsed server response: response = '%s', status = '%s'\n", response, status);

    // Handle server responses
    if (!strcmp(response, "RDB")) {
        if (!strcmp(status, "OK")) {
            printf("Debug mode started successfully!\n");
            printf("Time: %d seconds\n", time);
            printf("Secret key: %c %c %c %c\n", C1, C2, C3, C4);
            printf(DEBUG_STARTED, time, C1, C2, C3, C4);

            *trial_number = 1;     // Reset trial number
            *max_playtime = time;  // Set max playtime
        } else if (!strcmp(status, "NOK")) {
            printf(DEBUG_ONGOING);
        } else if (!strcmp(status, "ERR")) {
            printf(ERROR);
        } 
    } else {
        printf(FORMAT_ERROR);
    }
}

void quit(char* hostname, char* port, char *buffer, char *PLID, int *trial_number) {
    // Step 1: Format the QUT command
    sprintf(buffer, "QUT %s\n", PLID);

    // Step 2: Send the command and receive the server's response
    if (!sendAndReadUDP(buffer, hostname, port))
        return;

    // Step 3:  Parse the response
    char buf[4], n, key[16];
    int parsed = sscanf(buffer, "%s %s%c", buf, key, &n);

    // Case: RQT response
    if (parsed >= 2 && !strcmp(buf, "RQT")) {
        // Case: RQT OK
        if (sscanf(buffer, "RQT OK %15[^\n]", key) == 1) {
            *trial_number = 0; // Reset trial number for terminated game
            printf(QUIT, key);
            return;
        }
        // Case: RQT NOK
        if (!strcmp(key, "NOK")) {
            printf(NO_GAME);
            return;
        }
        // Case: RQT ERR
        if (!strcmp(key, "ERR")) {
            printf(ERROR);
            return;
        }
    } 
    // Case: ERR response
    else if (!strcmp(buf, "ERR")) {
        printf(ERROR);
        return;
    } 
    // Invalid response format
    printf(FORMAT_ERROR);
}


//------------------------------------- PLAYER TCP COMMANDS ---------------------------------------------//

void scoreboard(char* hostname, char* port, char *buffer){
    sprintf(buffer, "SSB\n");

    createTCPsocket(hostname, port);
    if(!sendAndReadTCP(buffer))
        return;

    if(!strcmp(buffer, "RSS")){
        if(!readUntilSpace(buffer))
            return;
        if(!strcmp(buffer, "OK")){
            readFile(0, buffer);
        } else if(!strcmp(buffer, "EMPTY")){
            printf(NO_SCORES);
        } else if(!strcmp(buffer, "ERR"))
            printf(ERROR);
        else {
            printf(FORMAT_ERROR);
        }
    } else if(!strcmp(buffer, "ERR"))
        printf(ERROR);
    else
        printf(FORMAT_ERROR);

    closeSocket();
}

void show_trials(char* hostname, char* port, char *buffer, char *PLID, int *trial_number){
    sprintf(buffer, "STR %s\n", PLID);

    createTCPsocket(hostname, port);
    if(!sendAndReadTCP(buffer))
        return;

    if(!strcmp(buffer, "RST")){
        if(!readUntilSpace(buffer))
            return;
        if(!strcmp(buffer, "ACT")){
            readFile(1, buffer);
        } else if(!strcmp(buffer, "FIN")){
            readFile(1, buffer);
            *trial_number = 0;
        } else if(!strcmp(buffer, "NOK")){
            printf(NO_STATE);
        } else if(!strcmp(buffer, "ERR"))
            printf(ERROR);
        else
            printf(FORMAT_ERROR);
    } else if(!strcmp(buffer, "ERR"))
        printf(ERROR);
    else {
        printf(FORMAT_ERROR);
    }
    closeSocket();

}


//------------------------------------- AUXILIAR COMMANDS ---------------------------------------------//

int validKey(char C1, char C2, char C3, char C4) {
    char validColors[] = {'R', 'G', 'B', 'Y', 'O', 'P'};
    char guess[] = {C1, C2, C3, C4};
    for (int i = 0; i < 4; i++) {
        int isValid = 0;
        for (int j = 0; j < 6; j++) {
            if (guess[i] == validColors[j]) {
                isValid = 1;
                break;
            }
        }
        if (!isValid) {
            return 0; // Invalid color found
        }
    }
    return 1; // All colors are valid
}

int validPLID(char *PLID) {
    if (strlen(PLID) != 6) return 0; // Must be exactly 6 characters
    for (int i = 0; i < 6; i++) {
        if (!isdigit(PLID[i])) return 0; // Must contain only digits
    }
    return 1;
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int readUntilSpace(char *ptr){
    do {
        n = read(fd, ptr, 1);
        if(n == -1){
            printf(RECEIVE_FAILED);
            return 0;
        }
        ptr+=n;
    } while(*(ptr-1)!=' ' && *(ptr-1)!='\n');
    *(ptr-1) = '\0';
    return 1;
}

int isNumeric(char *str){
    while(*str){
        if(!isdigit(*str))
            return 0;
        str++;
    }
    return 1;
}
