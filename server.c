#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

float get_cpu_load() {
    FILE *fp = popen("top -bn1 | grep \"Cpu(s)\" | awk '{print $2}'", "r");
    if (!fp) return -1;

    float load = -1;
    fscanf(fp, "%f", &load);
    pclose(fp);
    return load;
}


void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);  

    char buffer[BUFFER_SIZE] = {0};
    int bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);

    if (bytes <= 0) {
        close(client_socket);
        return NULL;
    }

    if (strncmp(buffer, "LOAD", 4) == 0) {
        float load = get_cpu_load();
        char response[50];
        snprintf(response, sizeof(response), "%.2f", load);
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return NULL;
    }

    printf("this server has received executable\n");
    
    FILE *fp = fopen("received_exec", "wb");
    if (!fp) {
        perror("File open failed");
        close(client_socket);
        return NULL;
    }

    fwrite(buffer, 1, bytes, fp);

    while ((bytes = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes, fp);
    }
    fclose(fp);

    int compile_status = system("gcc received_exec -o exec_bin 2> compile_error.txt");

    if (compile_status != 0) {
        system("chmod +x received_exec");
        system("./received_exec > output.txt 2>&1");
    } else {
        system("./exec_bin > output.txt 2>&1");
    }

    FILE *out = fopen("output.txt", "a");
    if (out) {
        fprintf(out, "\n[Executed in thread]\n");
        fclose(out);
    }

    fp = fopen("output.txt", "rb");
    if (!fp) {
        perror("Output open failed");
        close(client_socket);
        return NULL;
    }

    while ((bytes = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        send(client_socket, buffer, bytes, 0);
    }

    fclose(fp);
    close(client_socket);

    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int PORT = atoi(argv[1]);

    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    printf("Server running on port %d\n", PORT);

    while (1) {
        int *client_socket = malloc(sizeof(int));
        *client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);

        if (*client_socket < 0) {
            perror("Accept failed");
            free(client_socket);
            continue;
        }

        pthread_t tid;

        pthread_create(&tid, NULL, handle_client, client_socket);

        pthread_detach(tid);
    }

    return 0;
}
