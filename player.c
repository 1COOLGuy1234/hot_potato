#include "potato.h"

void clearSet(struct addrinfo *res1, struct addrinfo *res2, struct addrinfo *res3, int sockfd1, int sockfd2, int sockfd3) {
    freeaddrinfo(res1);
    freeaddrinfo(res2);
    freeaddrinfo(res3);
    close(sockfd1);
    close(sockfd2);
    close(sockfd3);
}

void throw_potato(int sockfd, int prev_sockfd, int next_sockfd, potato* my_potato, int player_id, int prev_player_id, int next_player_id) {
    my_potato->hops_num--;
    my_potato->path[my_potato->path_index] = player_id;
    my_potato->path_index++;
    if (my_potato->hops_num == 0) {
        printf("I'm it\n");
        mySend(sockfd, my_potato, sizeof(*my_potato), 0);
        return;
    }
    int random = rand() % 2;
    if (random == 0) {  // send to next
        mySend(next_sockfd, my_potato, sizeof(*my_potato), 0);
        printf("Sending potato to %d\n", next_player_id);
    }
    else { // send to prev
        mySend(prev_sockfd, my_potato, sizeof(*my_potato), 0);
        printf("Sending potato to %d\n", prev_player_id);
    }
}


int main(int argc, char** argv) {
    if (argc != 3) {
        printError("format: ./player <machine_name> <port_num>\n");
    }
    const char* machine_name = argv[1];
    const char* port_num = argv[2];
    int optival = 1;

    struct addrinfo hints;
    struct addrinfo *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    myGetAddrInfo(machine_name, port_num, &hints, &res);

    int sockfd;
    sockfd = mySocket(res->ai_family, res->ai_socktype, res->ai_protocol);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optival, sizeof(int));


    myConnect(sockfd, res->ai_addr, res->ai_addrlen);

    /******************communication*************/
    int players_num = 0;
    int hop_nums = 0;
    int player_id = 0;
    myRecv(sockfd, &players_num, sizeof(int), 0);
    myRecv(sockfd, &hop_nums, sizeof(int), 0);
    myRecv(sockfd, &player_id, sizeof(player_id), 0);
    printf("Connected as player %d out of %d total players\n", player_id, players_num);
    
    /********** send ready message *****************/
    char ready_message_to_ringmaster[MESSAGE_LENGTH];
    sprintf(ready_message_to_ringmaster, "Player %d is ready to play", player_id);
    size_t ready_message_length = strlen(ready_message_to_ringmaster);
    mySend(sockfd, &ready_message_length, sizeof(ready_message_length), 0);
    mySend(sockfd, ready_message_to_ringmaster, ready_message_length, 0);
    /**************************************************/

    /******send hostname*****/
    char hostname[NAME_LENGTH];
    myGetHostName(hostname, NAME_LENGTH);
    size_t hostname_length = strlen(hostname);
    mySend(sockfd, &hostname_length, sizeof(hostname_length), 0);
    mySend(sockfd, hostname, hostname_length, 0);
    /***********************/

    /*******************************server prepare info****************************/
    struct addrinfo hints_next;
    struct addrinfo *res_next;
    memset(&hints_next, 0, sizeof(hints_next));
    hints_next.ai_family = AF_UNSPEC;
    hints_next.ai_socktype = SOCK_STREAM;
    hints_next.ai_flags = AI_PASSIVE;

    int my_port = INIT_PORT_NUM + player_id;
    char my_port_str[PORT_LENGTH];
    sprintf(my_port_str, "%d", my_port);

    int sockfd_server = 0;  // as a server's socket

    struct sockaddr_storage their_addr_next;
    socklen_t addr_size_next = sizeof(their_addr_next);

    int sockfd_to_next = 0;
    /********************************************************************/

    /***************************set all players to listen state*************/
    myGetAddrInfo(NULL, my_port_str, &hints_next, &res_next);
    struct addrinfo *p;
    for(p = res_next; p != NULL; p = p->ai_next) {
        sockfd_server = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd_server < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(sockfd_server, SOL_SOCKET, SO_REUSEADDR, &optival, sizeof(int));

        if (bind(sockfd_server, p->ai_addr, p->ai_addrlen) < 0) {
            close(sockfd_server);
            continue;
        }
        break;
    }

    if (p == NULL) {
        printError("Failed to bind");
    }

    myListen(sockfd_server, BACK_LOG);
    /***************************************************************/

    /************As a client, build connection with prev player*********/
    char prev_hostname[NAME_LENGTH];
    size_t prev_hostname_length = 0;
    int prev_player_id = 0;
    myRecv(sockfd, &prev_hostname_length, sizeof(prev_hostname_length), 0);
    myRecv(sockfd, prev_hostname, prev_hostname_length, 0);
    myRecv(sockfd, &prev_player_id, sizeof(prev_player_id), 0);
    int prev_port = INIT_PORT_NUM + prev_player_id;
    char prev_port_str[PORT_LENGTH];
    sprintf(prev_port_str, "%d", prev_port);

    struct addrinfo hints_prev;
    struct addrinfo *res_prev;
    memset(&hints_prev, 0, sizeof(hints_prev));
    hints_prev.ai_family = AF_UNSPEC;
    hints_prev.ai_socktype = SOCK_STREAM;
    myGetAddrInfo(prev_hostname, prev_port_str, &hints_prev, &res_prev);    // problem here
    int sockfd_to_prev;
    sockfd_to_prev = mySocket(res_prev->ai_family, res_prev->ai_socktype, res_prev->ai_protocol);
    myConnect(sockfd_to_prev, res_prev->ai_addr, res_prev->ai_addrlen);
    setsockopt(sockfd_to_prev, SOL_SOCKET, SO_REUSEADDR, &optival, sizeof(int));
    /*******************************************************************/


    /***********先listen，再connect，再accept，player0的connect最后被accpet******/
    sockfd_to_next = myAccept(sockfd_server, (struct sockaddr *)&their_addr_next, &addr_size_next);
    setsockopt(sockfd_to_next, SOL_SOCKET, SO_REUSEADDR, &optival, sizeof(int));

    /**************get next player id*******/
    int next_player_id = 0;
    if (player_id == players_num - 1) {
        next_player_id = 0;
    }
    else {
        next_player_id = player_id + 1;
    }

    /********************test communication with prev && next player******************/
    char msg_to_prev[100];
    char msg_to_next[100];
    sprintf(msg_to_prev, "qqqqq");
    sprintf(msg_to_next, "qqqqq");

    /*******************************************************************/

    /******************************wait for the potato******/
    potato my_potato;
    memset(my_potato.path, 0, 512 * sizeof(int));
    srand((unsigned int) time(NULL) + player_id);

    fd_set readfds;
    int max_fd = 0;
    max_fd = (sockfd > sockfd_to_next) ? sockfd : sockfd_to_next;
    max_fd = (max_fd > sockfd_to_prev) ? max_fd : sockfd_to_prev;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(sockfd_to_next, &readfds);
        FD_SET(sockfd_to_prev, &readfds);
        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) == -1) {
            printError("Select has error");
        }
        if (FD_ISSET(sockfd, &readfds)) { // the first throw from ringmaster
            int recv_len = myRecv(sockfd, &my_potato, sizeof(my_potato), 0);
            if (recv_len == 0) {
                clearSet(res, res_next, res_prev, sockfd, sockfd_to_next, sockfd_to_prev);
                return EXIT_SUCCESS;
            }
            throw_potato(sockfd, sockfd_to_prev, sockfd_to_next, &my_potato, player_id, prev_player_id, next_player_id);
        }
        else if (FD_ISSET(sockfd_to_next, &readfds)) {
            int recv_len = myRecv(sockfd_to_next, &my_potato, sizeof(my_potato), 0);
            if (recv_len == 0) {
                clearSet(res, res_next, res_prev, sockfd, sockfd_to_next, sockfd_to_prev);
                return EXIT_SUCCESS;
            }
            throw_potato(sockfd, sockfd_to_prev, sockfd_to_next, &my_potato, player_id, prev_player_id, next_player_id);
        }
        else if (FD_ISSET(sockfd_to_prev, &readfds)) {
            int recv_len = myRecv(sockfd_to_prev, &my_potato, sizeof(my_potato), 0);
            if (recv_len == 0) {
                clearSet(res, res_next, res_prev, sockfd, sockfd_to_next, sockfd_to_prev);
                return EXIT_SUCCESS;
            }
            throw_potato(sockfd, sockfd_to_prev, sockfd_to_next, &my_potato, player_id, prev_player_id, next_player_id);
        }
        else {
            printError("select has error: no fd ready");
        }
    }
    return EXIT_FAILURE;
}