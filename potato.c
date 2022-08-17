#include "potato.h"

void printError(const char * msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(EXIT_FAILURE);
}


void myGetAddrInfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
    int status;
    if ((status = getaddrinfo(node, service, hints, res)) != 0) {
        printError("Get addr info has error");
    }
}

int mySocket(int domain, int type, int protocol) {
    int status;
    if ((status = socket(domain, type, protocol)) < 0) {
	    printError("Socket has error");
    }
    return status;
}

void myConnect(int sockfd, struct sockaddr *serv_addr, int addrlen) {
    int status;
    if ((status = connect(sockfd, serv_addr, addrlen)) < 0) {
    	printError("Connect has error");
    }
}

void myBind(int sockfd, struct sockaddr *my_addr, int addrlen) {
    int status;
    if ((status = bind(sockfd, my_addr, addrlen)) < 0) {
    	printError("Bind has error");
    }
}

void myListen(int sockfd, int backlog) {
    int status;
    if ((status = listen(sockfd,  backlog)) < 0) {
    	printError("Listen has error");
    }
}

int myAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int status;
    if ((status = accept(sockfd, addr, addrlen)) < 0) {
    	printError("Accept has error");
    }
    return status;
}

struct hostent *myGetHostByName(const char *name) {
    struct hostent *p;
    if ((p = gethostbyname(name)) == NULL) {
    	printError("Gethostbyname has error");
    }
    return p;
}

void myGetHostName(char* hostname, size_t size) {
    int status;
    if ((status = gethostname(hostname, size) < 0)) {
        printError("Gethostname has error");
    }
}

ssize_t mySend(int sockfd, const void* buf, size_t len, int flags) {
    ssize_t actual_len;
    if ((actual_len = send(sockfd, buf, len, flags)) == -1) {
        printError("Send has error");
    }
    return actual_len;
}

ssize_t myRecv(int sockfd, void* buf, size_t len, int flags) {
    ssize_t actual_len;
    if ((actual_len = recv(sockfd, buf, len, flags)) == -1) {
        printError("Recv has error");
    }
    return actual_len;
}




