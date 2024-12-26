#include "server_udp.h"
#include "server_tcp.h"
#include  <signal.h>

void INThandler(int sig);

int main (int argc, char** argv){
    
    int verbose = 0;
    char* port = "58064";       // 58000 + GN(=64)

    for(int i = 1; i < argc;){
        if(!strcmp(argv[i], "-p")){     // read port
            port = argv[i+1];
            i += 2;
        }
        else if(!strcmp(argv[i], "-v")){        // checks verbose
            verbose = 1;
            i += 1;
        }
        else{       // error
            fprintf(stderr, "error: invalid argument '%s'\n", argv[i]);
            exit(1);
        }
    }

    signal(SIGINT, INThandler);

    if(atoi(port) < 1024 || atoi(port) > 65535){
        fprintf(stderr, "error: invalid port number\n");
        exit(1);
    }

    struct sigaction act;
    memset(&act, 0, sizeof act);
    act.sa_handler = SIG_IGN;
    if(sigaction(SIGCHLD, &act, NULL)==-1){
        perror("sigaction");
        exit(1);
    }
    
    pid_t c1_pid, c2_pid, wpid;
    int status;


    (c1_pid = fork()) && (c2_pid = fork());
    
    if(c1_pid == 0){        // 1st child process
        udp_protocol(port, verbose);
        exit(0);
    }
    else if(c2_pid == 0){       // 2nd child process
        tcp_protocol(port, verbose);
        exit(0);
    }
    else if(c1_pid > 0 && c2_pid > 0){      // parent process
          while ((wpid = wait(&status)) > 0);
    }
    else{     // error
        perror("fork");
        exit(1);
    }
}

void INThandler(int sig){
    signal(sig, SIG_IGN);
    while(kill(0, SIGKILL)!=0);
    printf(BYE_SERVER);
    exit(0);
}