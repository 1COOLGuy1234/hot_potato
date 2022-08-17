#include "potato.h"


void print_init_info(int players_num, int hops_num) {
    printf("Potato Ringmaster\n");
    printf("Players = %d\n", players_num);
    printf("Hops = %d\n", hops_num);
}



int main(int argc, char** argv) {
    if (argc != 4) {
        printError("format: ./ringmaster <port_num> <players_num> <hops_num>\n");
    }

    potato this_potato;
    memset(this_potato.path, 0, 512 * sizeof(int));
    this_potato.players_num = atoi(argv[2]);
    this_potato.hops_num = atoi(argv[3]);
    this_potato.path_index = 0;

    if (this_potato.players_num <= 1) {
        printError("The players number must be greater than 1");
    }

    if (this_potato.hops_num < 0 || this_potato.hops_num > 512) {
        printError("The hops number must be in [0, 512]");
    }

    /*****getaddrinfo*****/
    struct addrinfo hints;
    struct addrinfo *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // server set this to bind

    myGetAddrInfo(NULL, argv[1], &hints, &res);  // server's node == NULL
    /*********************/

    /*******socket********/
    int sockfd;
    int optival = 1;
    /*********************/

    /*******bind**********/
    // myBind(sockfd, res->ai_addr, res->ai_addrlen);
    struct addrinfo *p;
    for(p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optival, sizeof(int));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            close(sockfd);
            continue;
        }
        break;
    }
    /*********************/

    /*******listen********/
    myListen(sockfd, BACK_LOG);
    /*********************/

    /*********print initial info********/
    print_init_info(this_potato.players_num, this_potato.hops_num);
    /************************************/

    /*******accept********/
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    int fd[this_potato.players_num];
    size_t ready_message_length = 0;
    char ready_message[MESSAGE_LENGTH];
    size_t hostname_length[this_potato.players_num];
    char hostname[this_potato.players_num][NAME_LENGTH];
    for (int i = 0; i < this_potato.players_num; i++) {
        fd[i] = myAccept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        mySend(fd[i], &this_potato.players_num, sizeof(this_potato.players_num), 0);
        mySend(fd[i], &this_potato.hops_num, sizeof(this_potato.hops_num), 0);
        mySend(fd[i], &i, sizeof(i), 0);

        myRecv(fd[i], &ready_message_length, sizeof(ready_message_length), 0);
        myRecv(fd[i], ready_message, ready_message_length, 0);

        myRecv(fd[i], &hostname_length[i], sizeof(hostname_length[i]), 0);
        myRecv(fd[i], hostname[i], hostname_length[i], 0);

        if (i > 0) {  // send hostname
            mySend(fd[i], &hostname_length[i-1], sizeof(hostname_length[i-1]), 0);
            mySend(fd[i], hostname[i-1], hostname_length[i-1], 0);
            int prev_id = i - 1;
            mySend(fd[i], &prev_id, sizeof(prev_id), 0);
        }
        printf("%s\n", ready_message);
    }
    /*********************/

    /********************last corner case******************/
    int last_id = this_potato.players_num - 1;
    mySend(fd[0], &hostname_length[this_potato.players_num - 1], sizeof(hostname_length[this_potato.players_num - 1]), 0);
    mySend(fd[0], hostname[this_potato.players_num - 1], hostname_length[this_potato.players_num - 1], 0);
    mySend(fd[0], &last_id, sizeof(last_id), 0);

    /******************************************************/

    /*********************start the game**************/
    if (this_potato.hops_num == 0) {   // firstly, observe if hops_num == 0
        freeaddrinfo(res);
        close(sockfd);
        return EXIT_SUCCESS;
    }
    srand((unsigned int) time(NULL));
    int init_player_id = rand() % this_potato.players_num;

    // randomly throw potato
    mySend(fd[init_player_id], &this_potato, sizeof(this_potato), 0);
    printf("Ready to start the game, sending potato to player %d\n", init_player_id);

    /*********************recycle potato*******************/
    fd_set readfds;
    int max_fd = 0;
    potato potato_recv;
    // use select IO listen all socket
    FD_ZERO(&readfds);
    for (int i = 0; i < this_potato.players_num; i++) {
        FD_SET(fd[i], &readfds);
        if (fd[i] > max_fd) {
            max_fd = fd[i];
        }
    }
    if (select(max_fd + 1, &readfds, NULL, NULL, NULL) == -1) { // wait until recv message
        printError("Select has error");
    }
    for (int i = 0; i < this_potato.players_num; i++) {
        if (FD_ISSET(fd[i], &readfds)) {
            myRecv(fd[i], &potato_recv, sizeof(this_potato), 0);
            if (potato_recv.hops_num != 0) {
                printf("hop num = %d", potato_recv.hops_num);
                continue;
            }
            printf("Trace of potato:\n");
            for (int j = 0; j < potato_recv.path_index; j++) {
                if (j != potato_recv.path_index - 1) {
                    printf("%d,", potato_recv.path[j]);
                }
                else {
                    printf("%d", potato_recv.path[j]);
                }
            }
            printf("\n");
            break;
        }
    }
    freeaddrinfo(res);
    close(sockfd);
    return EXIT_SUCCESS;
}