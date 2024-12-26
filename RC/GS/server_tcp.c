#include "server_tcp.h"

int fd_TCP, newfd_TCP, errno, errcode_TCP;
ssize_t n_TCP;
socklen_t addrlen_TCP;
struct addrinfo hints_TCP, *res_TCP;
struct sockaddr_in addr_TCP;

char buffer_TCP[MAX_FILE_SIZE + 128];

void tcp_protocol(char *port, int verbose){

    struct sigaction act;
    memset(&act, 0, sizeof act);
    act.sa_handler = SIG_IGN;
    if(sigaction(SIGPIPE, &act, NULL)==-1){
        perror("sigaction");
        exit(1);
    }

    TCP_OpenSocket(port); // Abre o socket TCP para escutar conexões
    while(1){
        
        addrlen_TCP = sizeof(addr_TCP);
        if((newfd_TCP = accept(fd_TCP, (struct sockaddr*)&addr_TCP, &addrlen_TCP)) == -1){
            printf("ERROR\n");
            exit(1);
        }

        pid_t c1_pid, wpid;

        c1_pid = fork(); // Cria um processo filho para tratar a conexão
        if(c1_pid == 0){
            // Processo filho
            char *ptr = buffer_TCP;
    
            do {
                n_TCP= read(newfd_TCP, ptr, 1);
                if(n_TCP== -1){
                    printf(RECEIVE_FAILED);
                    exit(1);
                }
                ptr+=n_TCP;
            } while(*(ptr-1)!=' ' && *(ptr-1)!='\n');
            *(ptr-1) = '\0';
            
            if(verbose){
                printf("----------------------\n");
                char host[NI_MAXHOST],service[NI_MAXSERV];

                if((errcode_TCP=getnameinfo((struct sockaddr *)&addr_TCP,addrlen_TCP,host,sizeof host,service,sizeof service,0))!=0) 
                    fprintf(stderr,"error: getnameinfo: %s\n",gai_strerror(errcode_TCP));
                else
                    printf("Sent by:\n\thost: %s\n\tport: %s\n", host, service);
                printf("Command: %s\n", buffer_TCP);
            }

            // Processa os comandos
            if(!strcmp(buffer_TCP, "SSB")){
                scoreboard(verbose);
            } else if(!strcmp(buffer_TCP, "STR")){
                show_trials(verbose);
            } else{
                // Comando inválido
                sprintf(buffer_TCP, "ERR\n");
                writeToTCP(buffer_TCP, 4, verbose);
            }
            if(verbose)
                printf("----------------------\n\n");

            close(newfd_TCP); // Fecha a conexão do cliente no processo filho
            exit(0);
        } else if(c1_pid < 0){
            printf("ERROR\n");
            exit(1);
        }
    }
    freeaddrinfo(res_TCP);
    close(fd_TCP); // Fecha o socket principal no processo pai
}

void TCP_OpenSocket(char *port){
    // Cria um socket TCP
    fd_TCP = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_TCP == -1){
        printf("ERROR\n");
        exit(1);
    }

    // Configura os parâmetros do socket
    memset(&hints_TCP, 0, sizeof hints_TCP);
    hints_TCP.ai_family = AF_INET;
    hints_TCP.ai_socktype = SOCK_STREAM;
    hints_TCP.ai_flags = AI_PASSIVE;

    // Resolve o endereço e porta
    errcode_TCP = getaddrinfo(NULL, port, &hints_TCP, &res_TCP);
    if(errcode_TCP!=0){ 
        printf("ERROR\n");
        exit(1);
    }

    // Permite reutilizar o endereço do socket
    if (setsockopt(fd_TCP, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
    
    // Associa o socket a um endereço e porta
    n_TCP= bind(fd_TCP, res_TCP->ai_addr, res_TCP->ai_addrlen);
    if(n_TCP == -1){
        if(errno==EADDRINUSE){
            printf(ADDRESS_USED);
        } else
            printf("ERROR\n");
        exit(1);
    }

    // Configura o socket para escutar conexões
    if(listen(fd_TCP, 5) == -1){
        printf("ERROR\n");
        exit(1);
    }
}

int writeToTCP(char *ptr, int to_write, int verbose){
    // Envia os dados em partes enquanto ainda há bytes para enviar
    while((n_TCP = write(newfd_TCP, ptr, to_write))!=0){
        // Se ocorrer um erro na escrita, exibe a mensagem de falha e retorna 0
        if(n_TCP == -1){
            printf(SEND_FAILED);
            return 0;
        } 
        // Avança o ponteiro e decrementa os bytes restantes
        ptr += n_TCP;
        to_write -= n_TCP;
    }
    return 1;// Sucesso
}

int writeFile(char *filename, char *folder, char *buffer_TCP, int verbose) {
    char filepath[MAX_FILENAME_SIZE + strlen(folder) + 1];
    sprintf(filepath, "%s%s", folder, filename);

    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file: %s\n", strerror(errno));
        sprintf(buffer_TCP, "ERR\n");
        writeToTCP(buffer_TCP, strlen(buffer_TCP), verbose);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);

    if (size > 2048) { // Limite de 2 KiB no protocolo
        fclose(fp);
        fprintf(stderr, "File size exceeds limit: %d bytes\n", size);
        sprintf(buffer_TCP, "ERR\n");
        writeToTCP(buffer_TCP, strlen(buffer_TCP), verbose);
        return 0;
    }

    // Envia o nome do arquivo e o tamanho
    sprintf(buffer_TCP, "%s %d ", filename, size);
    if (!writeToTCP(buffer_TCP, strlen(buffer_TCP), verbose)) {
        fclose(fp);
        return 0;
    }

    if (verbose)
        printf("Sent metadata: %s\n", buffer_TCP);

    size_t n;
    // Envia o conteúdo do arquivo
    while (size > 0) {
        n = fread(buffer_TCP, 1, MAX_READ_SIZE, fp);
        if (!writeToTCP(buffer_TCP, n, verbose)) {
            fclose(fp);
            return 0;
        }
        size -= n;
    }

    fclose(fp);

    if (verbose)
        printf("File '%s' sent successfully.\n", filename);

    return 1;
}

int isNumericTCP(char *str) {
    while (*str) {
        if (!isdigit(*str))
            return 0;
        str++;
    }
    return 1;
}

int readPLID(char *PLID, int verbose) {
    int n_left = MAX_PLID_SIZE;
    char *ptr = PLID;

    // Ler PLID do socket TCP
    while (n_left > 0) {
        n_TCP = read(newfd_TCP, ptr, n_left);
        if (n_TCP == -1) {
            printf(RECEIVE_FAILED);
            exit(1);
        }
        ptr += n_TCP;
        n_left -= n_TCP;
    }
    *ptr = '\0'; // Finalizar string

    // Ler último caractere para verificar final da mensagem
    n_TCP = read(newfd_TCP, buffer_TCP, 1);
    if (n_TCP <= 0 || buffer_TCP[0] != '\n' || strlen(PLID) != MAX_PLID_SIZE || !isNumericTCP(PLID)) {
        return 0;
    }

    if (verbose) {
        printf("PLID: %s\n", PLID);
    }
    return 1;
}

int findTopScores(char sb_file[MAX_FILE_SIZE + 1]) {
    struct dirent **filelist;
    int n_entries, i_file = 0, score, trials;
    char fname[SCORE_FILENAME_SIZE + 1], PLID[MAX_PLID_SIZE + 1], code[MAX_WORD_LENGTH + 1], mode[8];
    FILE *fp;

    n_entries = scandir(FOLDER_SCORES, &filelist, 0, alphasort);
    if (n_entries < 0) {
        return 0;
    }

    // Header
    strcpy(sb_file, "\n------------- TOP 10 SCORES -------------\n");
    strcat(sb_file, " SCORE    PLAYER   CODE   TRIALS   MODE\n");
    strcat(sb_file, "-----------------------------------------\n");

    while (n_entries-- > 0 && i_file < 10) {
        if (filelist[n_entries]->d_name[0] != '.') {
            char filename[MAX_FILENAME_SIZE + strlen(FOLDER_SCORES) + 1];
            sprintf(filename, "%s%s", FOLDER_SCORES, filelist[n_entries]->d_name);

            fp = fopen(filename, "r");
            if (fp != NULL) {
                fscanf(fp, "%d %s %s %d %s", &score, PLID, code, &trials, mode);
                fclose(fp);

                // Formatar cada linha com colunas centralizadas
                sprintf(buffer_TCP, "  %-7d %-8s %-8s %-8d %-8s\n", score, PLID, code, trials, mode);
                strcat(sb_file, buffer_TCP);

                i_file++;
            }
        }
        free(filelist[n_entries]);
    }
    free(filelist);

    strcat(sb_file, "-----------------------------------------\n");

    return i_file;
}

void scoreboard(int verbose) {
    char scoreboard_file[MAX_FILE_SIZE + 1];
    int num_scores = findTopScores(scoreboard_file);

    if (num_scores == 0) {
        // Caso não haja scores, enviar EMPTY
        sprintf(buffer_TCP, "RSS EMPTY\n");
    } else {
        // Caso haja scores, enviar o conteúdo formatado
        snprintf(buffer_TCP, sizeof(buffer_TCP), "RSS OK TOPSCORES_%07d.txt %zu %s\n", getpid(), strlen(scoreboard_file), scoreboard_file);
    }

    if (verbose) {
        printf("Sent to client: %s\n", buffer_TCP);
    }

    // Enviar o buffer ao cliente
    writeToTCP(buffer_TCP, strlen(buffer_TCP), verbose);
}

int findLastGame(char *PLID, char *filename){
    struct dirent *dir, **filelist;
    int n_entries, found;
    char dirname[strlen(FOLDER_GAMES) + MAX_PLID_SIZE + 2];


    sprintf(dirname, "%s%s/", FOLDER_GAMES, PLID);
    n_entries = scandir(dirname, &filelist, 0, alphasort);
    found = 0;

    if (n_entries <= 0)
        return (0);
    else{
        while (n_entries--){
            if (filelist[n_entries]->d_name[0] != '.'){
                char dname[MAX_FILENAME_SIZE + 1];
                strcpy(dname, filelist[n_entries]->d_name);
                sprintf(filename, "%s%s/%s", FOLDER_GAMES, PLID, dname);
                found = 1;
            }
            free(filelist[n_entries]);
            if (found == 1)
                break;
        }
        free(filelist);
    }
    return found;
}

void createTrialFile(char *PLID, char *filename, char file[MAX_FILE_SIZE + 1], int status) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        sprintf(file, "Error: Unable to open file %s\n", filename);
        return;
    }

    int n_trials = 0;
    char line[MAX_FILE_SIZE + 1], secret_code[MAX_WORD_LENGTH + 1];
    char mode_char, termination[16] = {0}, mode[6];
    char transactions[MAX_FILE_SIZE + 1] = {0};
    int remaining_time;
    char start_date[32], quit_date[32] = "", quit_time[16] = "";
    int start_time, duration = 0;
    int black, white, trial_time;
    char trial_code[MAX_WORD_LENGTH + 1];

    // Reading the first line (game details)
    if (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "%s %c %s %d %s %d", PLID, &mode_char, secret_code, &remaining_time, start_date, &start_time);
        strcpy(mode, (mode_char == 'P') ? "Play" : "Debug");
    }

    // Processing transactions
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] == 'T') {
            sscanf(line, "T: %s %d %d %d", trial_code, &black, &white, &trial_time);
            sprintf(line, "Trial: %s | nB: %d, nW: %d | Time: %d seconds\n", trial_code, black, white, trial_time);
            strcat(transactions, line);
            n_trials++;
        } else {
            // Parsing the last line for quit time and duration
            sscanf(line, "%s %s %d", quit_date, quit_time, &duration);
        }
    }

    // Constructing the file based on status
    file[0] = '\0';
    if (status == 1) { // Active game
        int remaining_time_actual = remaining_time - trial_time;
        if (remaining_time_actual < 0) remaining_time_actual = 0;

        sprintf(file, "Active game found for player %s\nGame initiated: %s with %ds to be completed\n", 
                PLID, start_date, remaining_time);
        strcat(file, "--- Transactions ---\n");
        if (n_trials > 0) {
            strcat(file, transactions);
        } else {
            strcat(file, "No transactions found.\n");
        }
        sprintf(line, "\n-- %d seconds remaining to be completed --\n", remaining_time_actual);
        strcat(file, line);

    } else { // Finalized game
        // Determining termination type
        sscanf(filename + strlen(FOLDER_GAMES) + MAX_PLID_SIZE + strlen("YYYYMMDD_HHMMSS_") + 1, "%c", &mode_char);
        if (mode_char == 'W')
            strcpy(termination, "WIN");
        else if (mode_char == 'F')
            strcpy(termination, "FAIL");
        else if (mode_char == 'Q')
            strcpy(termination, "QUIT");
        else if (mode_char == 'T')
            strcpy(termination, "Timeout");

        // Formatting the quit_at string
        char full_quit_at[64];
        sprintf(full_quit_at, "%s %s", quit_date, quit_time);

        sprintf(file, "Last finalized game for player %s\nGame initiated: %s with %ds to be completed\nMode: %s, Secret Code: %s\n\n", 
                PLID, start_date, remaining_time, mode, secret_code);
        strcat(file, "--- Transactions ---\n");
        if (n_trials > 0) {
            strcat(file, transactions);
        } else {
            strcat(file, "No transactions found.\n");
        }
        sprintf(line, "\nTermination: %s at %s, Duration: %ds\n", termination, full_quit_at, duration);
        strcat(file, line);
    }

    fclose(fp);
}

void show_trials(int verbose){
    char filename[MAX_FILENAME_SIZE + strlen(FOLDER_GAMES) + 1], state_filename[MAX_FILENAME_SIZE + 1];
    char PLID[MAX_PLID_SIZE+1], response[MAX_READ_SIZE+MAX_FILE_SIZE + 1];

    if(readPLID(PLID, verbose)){
        sprintf(filename, "%sGAME_%s.txt", FOLDER_GAMES, PLID);
        sprintf(state_filename, "STATE_%s.txt", PLID);
        char statefile[MAX_FILE_SIZE + 1];

        if(!access(filename, F_OK)){ 
            createTrialFile(PLID, filename, statefile, 1);
            sprintf(response, "RST ACT %s %zd %s\n", state_filename, strlen(statefile), statefile);
        }
        else {
            int found = findLastGame(PLID, filename);
            if (found){
                createTrialFile(PLID, filename, statefile, 0);
                sprintf(response, "RST FIN %s %zd %s\n", state_filename, strlen(statefile), statefile);
            }
            else
                sprintf(response, "RST NOK\n");
        }
    } else{
        sprintf(response, "RST ERR\n");
    }

    if(verbose)
        printf("Sent: %s\n", response);

    if(!writeToTCP(response, strlen(response), verbose))
        return;
}