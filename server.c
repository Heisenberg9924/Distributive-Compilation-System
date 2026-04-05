// simple_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {

    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    char buffer[BUFFER_SIZE];

    // 🔹 Step 1: Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 🔹 Step 2: Bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));

    // 🔹 Step 3: Listen
    listen(server_fd, 3);
    printf("Server listening...\n");

    // 🔹 Step 4: Accept connection
    client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    printf("Client connected\n");

    // 🔹 Step 5: Receive executable
    FILE *fp = fopen("received_exec", "wb");

    int bytes;
    while ((bytes = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes, fp);
        if (bytes < BUFFER_SIZE) break;
    }

    fclose(fp);
    printf("Received executable\n");

    // 🔹 Step 6: Give execute permission
    system("chmod +x received_exec");

    // 🔹 Step 7: Execute program
    system("./received_exec > output.txt");
    printf("Executed program\n");

    // 🔹 Step 8: Send output back
    fp = fopen("output.txt", "r");

    while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        send(client_socket, buffer, bytes, 0);
    }

    fclose(fp);

    printf("Output sent back\n");

    close(client_socket);
    close(server_fd);

    return 0;
}