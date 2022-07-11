#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define MAX_CONNECTIONS 100
#define MAX_EVENTS 100
#define MAX_LINE 4096
#define PORT 8080

void print_and_exit(char* message) {
    printf("%s", message);
    exit(1);
}

void handle_connection(int connfd) {
    char sendline[] = "HTTP/1.1 200 OK\n"
                      "Server: TestServer (Linux)\n"
                      "Content-Length: 88\n"
                      "Content-Type: text/html\n"
                      "Connection: Closed\r\n"
                      "<html>\n"
                      "<body>\n"
                      "<h1>Hello, World!</h1>\n"
                      "</body>\n"
                      "</html>\r\n\r\n";

    char recvline[MAX_LINE];
    ssize_t n;

    n = recv(connfd, recvline, MAX_LINE, AF_INET);
    printf("%s\n", recvline);

    bzero(recvline, n);

    if (send(connfd, sendline, MAX_LINE, AF_INET) < 0)
        print_and_exit("Error in send");
}

void accept_connections() {
    struct epoll_event ev, events[MAX_EVENTS];
    int listenfd, connfd, nfds, epollfd;
    struct sockaddr_in server, client;
    socklen_t c = sizeof(struct sockaddr_in);

    char buffer[MAX_LINE];
    char message[] = "Hello from epoll connections handler!";

    // Prepare sockets
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;

    // Create socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        print_and_exit("Error in socket!");

    // Bind to socket to address
    if (bind(listenfd, (struct sockaddr*) &server, sizeof server) < 0)
        print_and_exit("Error in bind!");

    listen(listenfd, MAX_CONNECTIONS);

    // epoll_create - legacy
    if ((epollfd = epoll_create1(0)) < 0)
        print_and_exit("Error in epoll_create");

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;

    // add listenfd to epoll interest list
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) < 0)
        print_and_exit("Error in epoll_ctl");

    char request[MAX_LINE];
    while (strstr(request, "close") == NULL) {
        // waits for new events and returns number of new events
        if ((nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1)) < 0)
            print_and_exit("Error in epoll_wait");

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == listenfd) {
                if ((connfd = accept(listenfd, (struct sockaddr*) &client, &c)) < 0)
                    print_and_exit("Error in accept");

                handle_connection(connfd);

//                ev.events = EPOLLIN;
//                ev.data.fd = connfd;

                // set file descriptor as non-blocking io
                if (fcntl(connfd, F_SETFL, (fcntl(connfd, F_GETFL)|O_NONBLOCK)) < 0)
                    print_and_exit("Error in fcntl");

                // add new connection to epoll interest list
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) < 0)
                    print_and_exit("Error in epoll_ctl");
            }
            else {
                close(events[i].data.fd);
            }
        }
    }

    close(connfd);
    for (int i = 0; i < sizeof(events); ++i)
        close(events[i].data.fd);

    close(epollfd);
}

int main() {
    printf("Waiting for connections...\n");
    accept_connections();
    return 0;
}

//void log_message(char* message) {
//    FILE* log_file = fopen("log.txt", "w");
//    fprintf(log_file, "%s\n", message);
//    fclose(log_file);
//}