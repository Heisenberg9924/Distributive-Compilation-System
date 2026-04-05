// simple_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024


int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Usage: %s <source_file.cpp> <server_ip>\n", argv[0]);
        return 1;
    }

    // 🔹 Step 1: Compile locally
    char cmd[200];
    sprintf(cmd, "g++ %s -o executable", argv[1]);
    int ret = system(cmd);
    if(ret != 0){
        printf("Compilation failed\n");
        return 1;
    }

    printf("Compiled successfully\n");

    // 🔹 Step 2: Choose server
    char* server_ip = argv[2];
    printf("Connecting to %s\n", server_ip);

    // 🔹 Step 3: Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    // 🔹 Step 4: Send executable
    FILE *fp = fopen("executable", "rb");
    if (!fp) {
        perror("File open failed");
        return 1;
    }
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
