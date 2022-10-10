#include <stdio.h>
#include <stdlib.h>         // exit, atoi
#include <string.h>         // memset
#include <unistd.h>         // sockaddr_in, read, write
#include <sys/socket.h>     // socket
#include <arpa/inet.h>      // struct sockaddr_in, htons, htonl, INADDR_ANY
#include <pthread.h>        // thread

void handleError(char *err);
// socket -> bind -> listen -> accept -> close
int create_server_socket();   
void server_bind(int serverSocket, char *port);
int accept_client(int serverSocket);

int main(int argc, char *argv[])
{

    int serverSocket = create_server_socket();

    /* bind */
    server_bind(serverSocket, argv[1]);

    /* listen */
    if(listen(serverSocket, 4) == -1) {  // maximum 4 clients
        handleError("Listen Error!");
    }

    /* accpet */
    int clientSocket = accept_client(serverSocket);

    char msg[] = "test success!\n";
    write(clientSocket, msg, sizeof(msg));

    close(serverSocket);
    close(clientSocket);
    return 0;
}

void handleError(char *err) 
{
    fprintf(stderr, "%s\n", err);
    exit(1);
}

// https://jhnyang.tistory.com/251
int create_server_socket()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == -1) {
        handleError("Socket Error!");
    }

    return serverSocket;
}

void server_bind(int serverSocket, char *port) 
{
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 ) {
        handleError("bind Error!");
    }
}

int accept_client(int serverSocket)
{
    int clientSocket;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    clientSocket = accept(serverSocket, (struct sockaddr*)&client_addr, &client_addr_size);
    if(clientSocket == -1) {
        handleError("Accept Error!");
    }

    return clientSocket;
}
    
