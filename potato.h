#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>


#define NAME_LENGTH 64
#define MESSAGE_LENGTH 128
#define INIT_PORT_NUM 50000
#define PORT_LENGTH 5
#define BACK_LOG 128
#define MAX_HOPS 512

/*****************struct***************/
struct potato_t {
    int players_num;
    int hops_num;
    int path[MAX_HOPS];
    int path_index;
};
typedef struct potato_t potato;
/*************************************/

/********************util***************************/
void printError(const char * msg);
/********************util***************************/

/*************************encapsulation*****************/
void myGetAddrInfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);

int mySocket(int domain, int type, int protocol);

void myConnect(int sockfd, struct sockaddr *serv_addr, int addrlen); 

void myBind(int sockfd, struct sockaddr *my_addr, int addrlen);

void myListen(int sockfd, int backlog); 

int myAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); 

struct hostent *myGetHostByName(const char *name);

void myGetHostName(char* hostname, size_t size);

ssize_t mySend(int sockfd, const void* buf, size_t len, int flags);

ssize_t myRecv(int sockfd, void* buf, size_t len, int flags);





/*************************encapsulation*****************/
