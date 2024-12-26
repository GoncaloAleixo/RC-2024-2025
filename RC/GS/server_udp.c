#include "server_udp.h"

int fd, errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;
char buffer[MAX_READ_SIZE + 1];


void udp_protocol (char* port, int verbose){
    char command[COMMAND_SIZE + 1];


    while(1){
        udp_connect(port);

        addrlen = sizeof(addr);
        n = recvfrom(fd, buffer, MAX_READ_SIZE, 0, (struct sockaddr*)&addr, &addrlen);
        if(n == -1){
            printf(RECEIVE_FAILED);
        }
        else{
            buffer[n] = '\0';

            sscanf(buffer, "%s", command);      // extracts the command
            if(!strcmp(command, "SNG")){
                start(verbose);
            } else if(!strcmp(command, "TRY")){
                try(verbose);
            } else if(!strcmp(command, "QUT")){
                quit(verbose);
            } else if(!strcmp(command, "DBG")){
                debug(verbose);
            } else{
                sprintf(buffer, "ERR");     // unexpected message received
                sendtoUDP();
            }
            freeaddrinfo(res);
            close(fd);
        }
    }
}

void udp_connect(char *port){
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1){
        printf("ERROR\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM;     // UDP socket
    hints.ai_flags = AI_PASSIVE;

    errcode = getaddrinfo (NULL, port, &hints, &res);
    if(errcode != 0){
        printf("ERROR\n");
        exit(1);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if(n == -1){
        if(errno==EADDRINUSE){
            printf(ADDRESS_USED);
        } else
            printf("ERROR\n");
        exit(1);
    }
}

void sendtoUDP(){
    n = sendto(fd, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, addrlen);
    if(n == -1){
        printf(SEND_FAILED);
    }
}

int validPlid(char *plid){
    if(strlen(plid) != PLID_SIZE){
        return 0;
    }
    for(int i=0; i<PLID_SIZE; i++){
        if(!isdigit(plid[i])){
            return 0;
        }
    }
    return 1;
}

int validTime(char *time){
    if(strlen(time) != TIME_SIZE || atoi(time) > MAX_TIME){
        return 0;
    }
    for(int i=0; i<TIME_SIZE; i++){
        if(!isdigit(time[i])){
            return 0;
        }
    }

    return 1;
} 

void generateSecretKey(char *secretKey){
    const char colours[] = {'R', 'G', 'B', 'Y', 'O', 'P'};
    const int numColours = sizeof(colours) / sizeof(colours[0]);

    srand((unsigned int)time(NULL));

    // Generate 4 random colours
    for(int i = 0; i < SECRET_KEY_SIZE; i++){
        secretKey[i] = colours[rand() % numColours];
    }

    // Null-terminate the string
    secretKey[SECRET_KEY_SIZE] = '\0';
}

void createGameFile(char *filename, char *plid, char mode, char *secretKey, char *maxTime){
    time_t startTime = time(NULL);
    struct tm tm = *localtime(&startTime);

    FILE *fp;
    fp = fopen(filename, "w");
    if(fp==NULL){
        perror("fopen");
        exit(1);
    }

    fprintf(fp, "%s %c %s %s %4d-%02d-%02d %02d:%02d:%02d %ld\n", plid, mode, secretKey, maxTime, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, startTime);
    fclose(fp);
}

char getGameMode(const char *filename){
    FILE *fp;
    fp = fopen(filename, "r");
    if(fp==NULL){
        perror("fopen");
        exit(1);
    }

    char mode;
    if(fgets(buffer, MAX_READ_SIZE, fp) != NULL) {
        sscanf(buffer, "%*s %c", &mode);
    } else {
        perror("Error reading file");
        fclose(fp);
        exit(1);
    }
    fclose(fp);
    return mode;
}

void getSecretKey (const char *filename, char *secretKey){
    FILE *fp;
    fp = fopen(filename, "r");
    if(fp==NULL){
        perror("fopen");
        exit(1);
    }

    int maxTime;
    if(fgets(buffer, MAX_READ_SIZE, fp) != NULL) {
        sscanf(buffer, "%*s %*s %s", secretKey);
    } else {
        perror("Error reading file");
        fclose(fp);
        exit(1);
    }
    fclose(fp);
}

int getStartTime(const char *filename){      // reads from GAME file
    FILE *fp;
    fp = fopen(filename, "r");
    if(fp==NULL){
        perror("fopen");
        exit(1);
    }

    int startTime;
    if(fgets(buffer, MAX_READ_SIZE, fp) != NULL) {
        sscanf(buffer, "%*s %*s %*s %*s %*s %*s %d", &startTime);
    } else {
        perror("Error reading file");
        fclose(fp);
        exit(1);
    }
    fclose(fp);
    return startTime;
    
}

int getMaxTime(const char *filename){
    FILE *fp;
    fp = fopen(filename, "r");
    if(fp==NULL){
        perror("fopen");
        exit(1);
    }

    int maxTime;
    if(fgets(buffer, MAX_READ_SIZE, fp) != NULL) {
        sscanf(buffer, "%*s %*s %*s %d", &maxTime);
    } else {
        perror("Error reading file");
        fclose(fp);
        exit(1);
    }
    fclose(fp);
    return maxTime;
}

int exceededTime(const char *filename){
    int maxTime = getMaxTime(filename);
    int startTime = getStartTime(filename);
    time_t currentTime = time(NULL);

    int duration = (int)(currentTime - startTime);
    if(duration > maxTime){
        return 1;
    }
    return 0;
}

int getExpectedTrial(const char *filename){
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1); 
    }

    int lineCount = 0; 
    while (fgets(buffer, MAX_READ_SIZE, fp) != NULL) {
        lineCount++;
    }
    fclose(fp);

    return lineCount;
}

void getPreviousKeyGuess(const char *filename, char *previousKeyGuess, int *nB, int *nW){
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1); 
    }

    while (fgets(buffer, MAX_READ_SIZE, fp) != NULL);

    sscanf(buffer, "%*s %s %d %d", previousKeyGuess, nB, nW);
}

int duration(const char *filename){
    int maxTime = getMaxTime(filename);
    int startTime = getStartTime(filename);
    time_t currentTime = time(NULL);

    int duration = (int)(currentTime - startTime);
    return duration;
}

void start(int verbose){
    char plid[PLID_SIZE + 1], n;
    char maxTime[TIME_SIZE + 1];

    int sscanfResult = sscanf(buffer + 4, "%s %s%c", plid, maxTime, &n);

    char verbose_buffer[MAX_READ_SIZE + 1];
    strcpy(verbose_buffer, buffer);
    
    char filename[MAX_FILENAME_SIZE + strlen(FOLDER_GAMES) + 1];
    sprintf(filename, "%sGAME_%s.txt", FOLDER_GAMES, plid);
    
    if(!access(filename, F_OK) && exceededTime(filename)){     // checks if time is exceeded
        finishGame(plid, filename, 'T');
    }

    if(sscanfResult != 3 || n!='\n' || !validPlid(plid) || !validTime(maxTime)){  // checks syntax
        sprintf(buffer, "RSG ERR\n");
        if(verbose){
            printVerbose(verbose_buffer, NULL, NULL, -1);
        }
    } else{    
        if(!access(filename, F_OK)){        // check if game file exists (ongoing game)
            sprintf(buffer, "RSG NOK\n");
            if(verbose){
            printVerbose(verbose_buffer, plid, NULL, -1);
            }
        } else{
            char mode = 'P';
            char secretKey[SECRET_KEY_SIZE + 1];
            generateSecretKey(secretKey);      // generates a random secret key
            createGameFile(filename, plid, mode, secretKey, maxTime);     // writes first line
            sprintf(buffer, "RSG OK\n");
            if(verbose){
            printVerbose(verbose_buffer, plid, NULL, -1);
            }
        }
    }
    sendtoUDP();
}

void secretKey_format(char *secretKeyGuess, char C1, char C2, char C3, char C4){

    secretKeyGuess[0] = C1;
    secretKeyGuess[1] = C2;
    secretKeyGuess[2] = C3;
    secretKeyGuess[3] = C4;
    secretKeyGuess[4] = '\0';

}

void keyOutput_format(const char *filename, char *keyOutput){
    char secretKey[SECRET_KEY_SIZE + 1];
    getSecretKey(filename, secretKey);

    keyOutput[0] = secretKey[0];
    keyOutput[1] = ' ';
    keyOutput[2] = secretKey[1];
    keyOutput[3] = ' ';
    keyOutput[4] = secretKey[2];
    keyOutput[5] = ' ';
    keyOutput[6] = secretKey[3];
    keyOutput[7] = '\0';

}

int validSecretKey(char *secretKeyGuess){
    if(strlen(secretKeyGuess) != SECRET_KEY_SIZE){
        return 0;
    }
    for(int i = 0; i < SECRET_KEY_SIZE; i++){
        if(secretKeyGuess[i]!='R' && secretKeyGuess[i]!='G' &&
           secretKeyGuess[i]!='B' && secretKeyGuess[i]!='Y' &&
           secretKeyGuess[i]!='O' && secretKeyGuess[i]!='P'){
            return 0;
        }
    }
    return 1;
}

int repeatSecretKey(char *filename, int nT, char *secretKeyGuess){
    if(nT < 2){
        return 0;
    }
    char previousKeyGuess[SECRET_KEY_SIZE + 1];

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1); 
    }

    while (fgets(buffer, MAX_READ_SIZE, fp) != NULL){
        sscanf(buffer, "%*s %s", previousKeyGuess);
        if(!strcmp(secretKeyGuess, previousKeyGuess)){
            return 1;
        }
    }
    return 0;
}

void finishGame(char *plid, char *filename, char code){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // write last list
    FILE *fp = fopen(filename, "a");
    if(fp==NULL){
        perror("fopen");
        exit(1);
    }
    fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d %d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, duration(filename));
    fclose(fp);

    char newFilename[MAX_FILENAME_SIZE + strlen(FOLDER_GAMES) + PLID_SIZE + 2];
    sprintf(newFilename, "%s%s", FOLDER_GAMES, plid);

    DIR* dir = opendir(newFilename);
    if(dir){
        closedir(dir);
    } else if(ENOENT == errno){
        mkdir(newFilename, 0777);
    } else{
        perror("opendir");
        exit(1);
    }
    sprintf(newFilename, "%s%s/%04d%02d%02d_%02d%02d%02d_%c.txt", FOLDER_GAMES, plid, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, code);
    rename(filename, newFilename);
}

void evaluateGuess(char *secretKeyGuess, char *secretKey, int *nB, int *nW){
    // Ensure these values are initialized properly
    *nB = 0;
    *nW = 0;

    bool counted_secretKey[SECRET_KEY_SIZE] = {false};
    bool counted_guessKey[SECRET_KEY_SIZE] = {false};


    for(int i = 0; i < SECRET_KEY_SIZE; i++){
        if(secretKeyGuess[i] == secretKey[i]){
            (*nB)++;
            counted_secretKey[i] = true;
            counted_guessKey[i] = true;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (!counted_guessKey[i]) {     // only consider unmatched guess positions
            for (int j = 0; j < 4; j++) {
                if (!counted_secretKey[j] && secretKeyGuess[i] == secretKey[j]) {
                    (*nW)++;
                    counted_secretKey[j] = true;        // mark this position as matched
                    break;
                }
            }
        }
    }
}

void addTrial(char *filename, char *secretKeyGuess, int nB, int nW){
    FILE *fp = fopen(filename, "a");
    if(fp==NULL){
        perror("fopen");
        exit(1);
    }

    fprintf(fp, "T: %s %d %d %d\n", secretKeyGuess, nB, nW, duration(filename));
    fclose(fp);
}

int calculateScore(int nT, int duration){
    double trialScore = ((double)(MAX_TRIALS - nT) / (MAX_TRIALS - 1)) * 50;
    double timeScore = ((double)(MAX_TIME - duration) / MAX_TIME) * 50;

    // total score is the sum of trial and time scores, rounded
    int totalScore = (int)(trialScore + timeScore);

    // ensure score is between 001 and 100
    if (totalScore < 1) totalScore = 1;
    if (totalScore > 100) totalScore = 100;

    return totalScore;
}

void createScoreFile(int score, char *plid, char *secretKey, int nT, char mode){
    char filename[SCORE_FILENAME_SIZE + 1];
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    sprintf(filename, "%s_%s_%04d%02d%02d_%02d%02d%02d.txt", FOLDER_SCORES, plid, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec); // create file in SCORES folder
    
    FILE* fp = fopen(filename, "w");
    if(mode == 'P'){
        fprintf(fp, "%03d %s %s %d PLAY\n", score, plid, secretKey, nT);      // mode = DEBUG
    }
    else{
        fprintf(fp, "%03d %s %s %d DEBUG\n", score, plid, secretKey, nT);       // mode = PLAY 
    }
    fclose(fp);
}

void try(int verbose){
    char plid[PLID_SIZE + 1];
    char C1, C2, C3, C4, n;
    int nT;

    int sscanfResult = sscanf(buffer + 4, "%s %c %c %c %c %d%c", plid, &C1, &C2, &C3, &C4, &nT, &n); 

    char verbose_buffer[MAX_READ_SIZE + 1];
    strcpy(verbose_buffer, buffer);

    char filename[MAX_FILENAME_SIZE + strlen(FOLDER_GAMES) + 1];
    sprintf(filename, "%sGAME_%s.txt", FOLDER_GAMES, plid);


    if(!access(filename, F_OK) && exceededTime(filename)){     // checks if time is exceeded
        char keyOutput[OUTPUT_KEY_SIZE + 1];
        keyOutput_format(filename, keyOutput);
        finishGame(plid, filename, 'T');
        sprintf(buffer, "RTR ETM %s\n", keyOutput);

        if(verbose){
            printVerbose(verbose_buffer, plid, NULL, -1);
        }

        sendtoUDP();
        return;
    } 
    
    char secretKeyGuess[SECRET_KEY_SIZE + 1];
    secretKey_format(secretKeyGuess, C1, C2, C3, C4);
    char secretKey[SECRET_KEY_SIZE + 1];
    getSecretKey(filename, secretKey);
    
    if(sscanfResult != 7 || n!='\n' || !validPlid(plid) || !validSecretKey(secretKeyGuess)){     // syntax
        sprintf(buffer, "RTR ERR\n");
        if(verbose){
            printVerbose(verbose_buffer, NULL, NULL, -1);
        }
    } else if(access(filename, F_OK)){        // check PLID having an ongoing game
        sprintf(buffer, "RTE NOK\n");
        if(verbose){
            printVerbose(verbose_buffer, plid, NULL, -1);
        }
    } else if(nT != getExpectedTrial(filename)){
        char previousKeyGuess[SECRET_KEY_SIZE + 1];
        int nB, nW;
        getPreviousKeyGuess(filename, previousKeyGuess, &nB, &nW);
        if(nT == getExpectedTrial(filename) - 1 && !strcmp(secretKeyGuess, previousKeyGuess)){       // resend
            sprintf(buffer, "RTR OK %d %d %d\n", nT, nB, nW) ;   
            if(verbose){
                printVerbose(verbose_buffer, plid, secretKeyGuess, nT);
            }
        } 
        else{       // invalid trial number
            sprintf(buffer, "RTR INV\n");
            if(verbose){
                printVerbose(verbose_buffer, plid, NULL, -1);
            }
        }    
    } else if(repeatSecretKey(filename, nT, secretKeyGuess)){     // duplicate key guess
        sprintf(buffer, "RTR DUP\n");
        if(verbose){
            printVerbose(verbose_buffer, plid, NULL, -1);
        }
    } else if(nT == 8 && strcmp(secretKeyGuess, secretKey)){       // no more moves
        char keyOutput[OUTPUT_KEY_SIZE + 1];
        keyOutput_format(filename, keyOutput);
        finishGame(plid, filename, 'F');
        sprintf(buffer, "RTR ENT %s\n", keyOutput);
        if(verbose){
            printVerbose(verbose_buffer, plid, NULL, -1);
        }
    } else{     // default
        int nB, nW;
        evaluateGuess(secretKeyGuess, secretKey, &nB, &nW);
        addTrial(filename, secretKeyGuess, nB, nW);

        if(nB == 4){     // checks is the game was won
            int score = calculateScore(nT, duration(filename));
            char mode = getGameMode(filename);

            finishGame(plid, filename, 'W');
            createScoreFile(score, plid, secretKey, nT, mode);
        }
        sprintf(buffer, "RTR OK %d %d %d\n", nT, nB, nW);

        if(verbose){
            printVerbose(verbose_buffer, plid, secretKeyGuess, nT);
        }
    }
    sendtoUDP();
}

void quit(int verbose){
    char plid[PLID_SIZE + 1], n;

    int sscanfResult = sscanf(buffer + 4, "%s%c", plid, &n);

    char verbose_buffer[MAX_READ_SIZE + 1];
    strcpy(verbose_buffer, buffer);

    char filename[MAX_FILENAME_SIZE + strlen(FOLDER_GAMES) + 1];
    sprintf(filename, "%sGAME_%s.txt", FOLDER_GAMES, plid);

    if(!access(filename, F_OK) && exceededTime(filename)){     // checks if time is exceeded
        finishGame(plid, filename, 'T');
    }
    if(sscanfResult!=2 || n!='\n' || !validPlid(plid)){     // checks syntax
        sprintf(buffer, "RQT ERR\n");
        if(verbose){
            printVerbose(verbose_buffer, NULL, NULL, -1);
        }
    } else{
        if(!access(filename, F_OK)){        // if game file exist finish game
            char keyOutput[OUTPUT_KEY_SIZE + 1];
            keyOutput_format(filename, keyOutput);
            finishGame(plid, filename, 'Q');
            sprintf(buffer, "RQT OK %s\n", keyOutput);
            if(verbose){
                printVerbose(verbose_buffer, plid, NULL, -1);
            }
        }
        else{       // game file doesn't exist (no ongoing game)
            sprintf(buffer, "RQT NOK\n");
            if(verbose){
                printVerbose(verbose_buffer, plid, NULL, -1);
            }
        }
    }
    
    sendtoUDP();
}

void debug(int verbose){
    char plid[PLID_SIZE + 1], C1, C2, C3, C4, n;
    char maxTime[TIME_SIZE + 1];

    int sscanfResult = sscanf(buffer + 4, "%s %s %c %c %c %c%c", plid, maxTime, &C1, &C2, &C3, &C4, &n);

    char verbose_buffer[MAX_READ_SIZE + 1];
    strcpy(verbose_buffer, buffer);
    
    char filename[MAX_FILENAME_SIZE + strlen(FOLDER_GAMES) + 1];
    sprintf(filename, "%sGAME_%s.txt", FOLDER_GAMES, plid);

    if(!access(filename, F_OK) && exceededTime(filename)){     // checks if time is exceeded
        char filename[MAX_FILENAME_SIZE + strlen(FOLDER_GAMES) + 1];
        sprintf(filename, "%sGAME_%s.txt", FOLDER_GAMES, plid);
        finishGame(plid, filename, 'T');
    }

    char secretKey[SECRET_KEY_SIZE + 1];
    secretKey_format(secretKey, C1, C2, C3, C4);        // translation into string
    
    if(sscanfResult != 7 || n!='\n' || !validPlid(plid) || !validTime(maxTime) || !validSecretKey(secretKey)){  // checks syntax
        sprintf(buffer, "RDB ERR\n");
        if(verbose){
            printVerbose(verbose_buffer, NULL, NULL, -1);
        }
    } else{
        
        if(!access(filename, F_OK)){        // check if file exists (ongoing game)
            sprintf(buffer, "RDB NOK\n");
            if(verbose){
            printVerbose(verbose_buffer, plid, NULL, -1);
            }
        }
        else{       // default
            char mode = 'D';
            createGameFile(filename, plid, mode, secretKey, maxTime);     // creates file and writes first line
            sprintf(buffer, "RDB OK\n");
            if(verbose){
                printVerbose(verbose_buffer, plid, NULL, -1);
            }
        }
    }
    sendtoUDP();
}

void printVerbose(char *verbose_buffer, char *plid, char *secretKeyGuess, int nT){
    char host[NI_MAXHOST],service[NI_MAXSERV];

    printf("----------------------\n");

    if((errcode=getnameinfo((struct sockaddr *)&addr,addrlen,host,sizeof host,service,sizeof service,0))!=0) 
        fprintf(stderr,"error: getnameinfo: %s\n",gai_strerror(errcode));
    else
        printf("Sent by:\n\thost: %s\n\tport: %s\n\n", host, service);
        
    printf("Receive: %s\n", verbose_buffer);
    if(plid!=NULL)
        printf("PLID: %s\n", plid);
    if(secretKeyGuess!=NULL)
        printf("Guess: %s\n", secretKeyGuess);
    if(nT!=-1)
        printf("Trial number: %d\n", nT);

    printf("Sent: %s\n", buffer);
    printf("----------------------\n");
} 


