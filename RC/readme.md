Projeto RC 2024/2025 - Master Mind Game

    ist1106900 - Gonçalo Aleixo
    ist1106937 - André Melo


Ficheiro Makefile e como Compilar:

    make - Para compilar o projeto basta executar o comando make dentro da diretoria /RC_Word_Game. Este comando irá criar dois executáveis (um para o Server dentro da diretoria /GS, e outro para o player dentro da diretoria /client).
    make clean - O comando make clean irá apagar todos os executáveis criados pelo comando make.
    make clear - O comando make clear irá apagar todos os ficheiros .txt do servidor.

Como executar:

    Player: dentro da diretoria do cliente (/RC_Word_Game/client) executar o comando ./player [-n GSIP] [-p GSport]. Os dois argumentos são opcionais, e correspondem ao IP onde é corrido o server e ao port do mesmo, respetivamente.
    Server: dentro da diretoria raiz (RC_Word_Game, É IMPORTANTE QUE SEJA EXECUTADO DENTRO DESTA DIRETORIA) executar o comando ./GS/server [-p GSport] [-v].O primeiro elemento e o segundo elemento são opcionais, correspondendo ao port do server e à opção de verbose.



Foi adicionado um timeout aos sockets do player para que, em caso de erro, o cliente não se mantivesse à espera de resposta infinitamente. (20 segundos)