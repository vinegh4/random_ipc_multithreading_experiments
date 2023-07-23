#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <string.h>
#include <errno.h>

#define SERVER_SOCK_PATH "unix_sock.server"
#define CLIENT_SOCK_PATH "unix_sock.client"
#define SERVER_MSG "HELLO FROM SERVER"
#define CLIENT_MSG "HELLO FROM CLIENT"

int main(int argc, char **argv) {
    struct sockaddr_un server_addr;
    struct sockaddr_un client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    int rc;
    char buf[256];

    pid_t pid = fork();

    if(pid != 0) {//parent process - server in this case
        int backlog = 10;

        int server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if(server_sock == -1) {
            printf("SERVER: error when opening server socket. \n");
            exit(1);
        }

        //bind server socket to address (file because its a unix socket)
        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, SERVER_SOCK_PATH);
        int len = sizeof(server_addr);

        //deletes file if it exists
        unlink(SERVER_SOCK_PATH);

        rc = bind(server_sock, (struct sockaddr *)&server_addr, len);
        if(rc == -1) {
            printf("SERVER: Server bind error: %s\n", strerror(errno));
            close(server_sock);
            exit(1);
        }

        //listen and accept connection from client
        rc = listen(server_sock, backlog);
        if(rc == -1) {
            printf("SERVER: Listen error: %s\n", strerror(errno));
            close(server_sock);
            exit(1);
        }

        printf("SERVER: Socket listening ...\n");
        int client_fd = accept(server_sock, (struct sockaddr *)&client_addr, (socklen_t *)&len);
        if(client_fd == -1) {
            printf("SERVER: Accept error: %s\n", strerror(errno));
            close(server_sock);
            close(client_fd);
            exit(1);
        }
        printf("SERVER: connected to client at: %s\n", client_addr.sun_path);
        printf("SERVER: Waiting for message...\n");

        //listen to client
        memset(buf, 0, 256);
        int byte_recv = recv(client_fd, buf, sizeof(buf), 0);
        if (byte_recv == -1) {
            printf("SERVER: error when receiving message: %s\n", strerror(errno));
            close(server_sock);
            close(client_fd);
            exit(1);
        }
        else {
            printf("SERVER: Server received message: %s.\n", buf);
        }

        //respond
        printf("SERVER: Respond to client...\n");
        memset(buf, 0, 256);
        strcpy(buf, SERVER_MSG);
        rc = send(client_fd, buf, strlen(buf), 0);
        if(rc == -1) {
            printf("SERVER: Error when sending message to client.\n");
            close(server_sock);
            close(client_fd);
            exit(1);
        }
        printf("SERVER: Done!\n");

        close(server_sock);
        close(client_fd);
        unlink(SERVER_SOCK_PATH);
    } 

    //child process - client in this case
    else {
        int client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (client_sock == -1) {
            printf("CLIENT: Socket error: %s\n", strerror(errno));
            exit(1);
        }

        //bind client socket to address
        client_addr.sun_family = AF_UNIX;
        strcpy(client_addr.sun_path, CLIENT_SOCK_PATH);
        int len = sizeof(client_addr);

        unlink(CLIENT_SOCK_PATH);
        rc = bind(client_sock, (struct sockaddr *)&client_addr, len);
        if(rc == -1) {
            printf("CLIENT: Client binding error. %s\n", strerror(errno));
            close(client_sock);
            exit(1);
        }

        //set server address and connect
        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, SERVER_SOCK_PATH);
        rc = connect(client_sock, (struct sockaddr *)&server_addr, len);
        if(rc == -1) {
            printf("CLIENT: Connect error. %s\n", strerror(errno));
            close(client_sock);
            exit(1);
        }
        printf("CLIENT: Connected to server.\n");

        //send message
        memset(buf, 0, sizeof(buf));
        strcpy(buf, CLIENT_MSG);
        rc = send(client_sock, buf, sizeof(buf), 0);
        if(rc == -1) {
            printf("CLIENT: Send error. %s\n", strerror(errno));
            close(client_sock);
            exit(1);
        }
        printf("CLIENT: Sent a message to server.\n");

        printf("CLIENT: Wait for response from server...\n");
        memset(buf, 0, sizeof(buf));
        rc = recv(client_sock, buf, sizeof(buf), 0);
        if (rc == -1) {
            printf("CLIENT: Recv Error. %s\n", strerror(errno));
            close(client_sock);
            exit(1);
        }
        else {
            printf("Client: Message received: %s\n", buf);
        }

        close(client_sock);
        unlink(CLIENT_SOCK_PATH);
    }

    return 0;
}