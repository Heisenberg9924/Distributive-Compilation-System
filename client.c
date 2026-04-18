#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFFER_SIZE 1024

float get_worker_load(char *ip, int port) {
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        return 9999;
    }

    send(sock, "LOAD", 4, 0);

    char buffer[50] = {0};
    recv(sock, buffer, sizeof(buffer), 0);

    close(sock);

    return atof(buffer);
}

int connect_to_worker(char *ip, int port) {
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        return -1;
    }

    return sock;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <source_file.c>\n", argv[0]);
        return 1;
    }

    char compile_cmd[200];
    char exec_name[100] = "program";  

    
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc %s -o %s 2> compile_error.txt", argv[1], exec_name);

    int status = system(compile_cmd);

    if (status != 0) {
        printf("Compilation failed. Check compile_error.txt\n");
        return 1;
    }

    char *workers[] = {"172.16.12.159", "172.16.13.207","172.16.12.153","172.16.12.156"};
    int ports[] = {8080, 8080,8080,8080};

    float min_load = 9999;
    int best = -1;

    for (int i = 0; i < 4; i++) {
        float load = get_worker_load(workers[i], ports[i]);
        printf("Server %d load: %.2f\n", ports[i], load);

        if (load < min_load) {
            min_load = load;
            best = i;
        }
    }

    if (best == -1 || min_load == 9999) {
        printf("No available Servers!\n");
        return 1;
    }

    printf("Selected Server at port %d\n", ports[best]);

    int sock = connect_to_worker(workers[best], ports[best]);

    FILE *fp = fopen(exec_name, "rb");
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

    shutdown(sock, SHUT_WR);

    printf("Output from server:\n");

    while ((bytes = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        write(1, buffer, bytes);
    }

    printf("\n");

    close(sock);

    return 0;
}
