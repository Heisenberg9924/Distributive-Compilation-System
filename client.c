// simple_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Simple Round Robin
char* servers[] = {"127.0.0.1", "127.0.0.1"};
int current = 0;

char* get_server() {
    char* s = servers[current];
    current = (current + 1) % 2;
    return s;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: %s file.cpp\n", argv[0]);
        return 1;
    }

    // 🔹 Step 1: Compile locally
    char cmd[200];
    sprintf(cmd, "g++ %s -o executable", argv[1]);
    system(cmd);

    printf("Compiled successfully\n");

    // 🔹 Step 2: Choose server
    char* server_ip = get_server();
    printf("Connecting to %s\n", server_ip);

    // 🔹 Step 3: Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    // 🔹 Step 4: Send executable
    FILE *fp = fopen("executable", "rb");
    char buffer[BUFFER_SIZE];

    int bytes;
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        send(sock, buffer, bytes, 0);
    }

    fclose(fp);
    printf("Sent executable\n");

    // 🔹 Step 5: Receive output
    printf("Output from server:\n");

    while ((bytes = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        write(1, buffer, bytes);
    }
    printf("\n");

    close(sock);

    return 0;
}