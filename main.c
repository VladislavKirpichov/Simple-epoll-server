#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAX_CONNECTIONS 100
#define MAX_LINE 4096

void print_and_exit(char* message) {
    printf("%s", message);
    exit(1);
}

void* connection_handler(void* connfd) {
    // printf("hello!\n");
    int fd = *((int*)connfd);
    free(connfd);

    char sendline[] = "HTTP/1.1 200 OK\n"
                      "Server: TestServer (Linux)\n"
                      "Content-Length: 88\n"
                      "Content-Type: text/html\n"
                      "Connection: Closed\r\n\r\n"
                      "<html>\n"
                      "<body>\n"
                      "<h1>Hello, World!</h1>\n"
                      "</body>\n"
                      "</html>";

    char recvline[MAX_LINE];
    struct sockaddr_in server;
    int sockfd;
    ssize_t n;

    n = recv(fd, recvline, MAX_LINE, AF_INET);
    printf("%s\n", recvline);

    send(fd, sendline, MAX_LINE, AF_INET);
    return NULL;
}

void accept_connections() {
    int connfd, listenfd;
    struct sockaddr_in server, client;

    // Prepare sockaddr struct
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);
    server.sin_family = AF_INET;

    // Create socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        print_and_exit("Error in socket!");

    // Bind to connfd
    if (bind(listenfd, (struct sockaddr*) &server, sizeof server) < 0)
        print_and_exit("Error in bind!");

    listen(listenfd, MAX_CONNECTIONS);

    // Accept clients
    socklen_t c = sizeof(struct sockaddr_in);
    while ((connfd = accept(listenfd, (struct sockaddr*) &client, &c)) > 0) {
        pthread_t t;

        int* new_sock = malloc(sizeof(int));
        *new_sock = connfd;
        pthread_create(&t, NULL, &connection_handler, (void*)new_sock);
    }
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