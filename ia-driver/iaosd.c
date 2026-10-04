/*
 * iaosd - IA Driver for JuliOS
 * Crea /dev/ia y comunica con Ollama
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DEVICE_PATH "/dev/ia"
#define OLLAMA_HOST "10.0.2.2"  // Host IP desde QEMU
#define OLLAMA_PORT 11434

/* Crear device node */
static void create_device(void) {
    unlink(DEVICE_PATH);
    if (mknod(DEVICE_PATH, S_IFCHR | 0666, makedev(1, 9)) == -1) {
        // Device 1,9 es /dev/console (mock)
        // Usamos otro numero menor
        if (mknod(DEVICE_PATH, S_IFCHR | 0666, makedev(100, 0)) == -1) {
            perror("mknod");
            exit(1);
        }
    }
    printf("[iaosd] Device %s creado\n", DEVICE_PATH);
}

/* Conectar a Ollama */
static int connect_ollama(void) {
    int sock;
    struct hostent *server;
    struct sockaddr_in serv_addr;

    server = gethostbyname(OLLAMA_HOST);
    if (server == NULL) {
        // Intentar con IP directa
        server = gethostbyname("host.docker.internal");
        if (server == NULL) {
            fprintf(stderr, "[iaosd] No se pudo resolver %s\n", OLLAMA_HOST);
            return -1;
        }
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(OLLAMA_PORT);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    printf("[iaosd] Conectado a Ollama en %s:%d\n", OLLAMA_HOST, OLLAMA_PORT);
    return sock;
}

/* Reenviar request a Ollama y retornar respuesta */
static int forward_to_ollama(int ollama_fd, const char *prompt, char *response, size_t resp_size) {
    char request[8192];
    int len;

    // Construir request simple
    len = snprintf(request, sizeof(request),
        "{\"model\": \"minimax-m2.5:cloud\", \"prompt\": \"%s\", \"stream\": false}\n",
        prompt);

    if (write(ollama_fd, request, len) < 0) {
        perror("write to ollama");
        return -1;
    }

    // Leer respuesta
    memset(response, 0, resp_size);
    int total = 0;
    char buf[512];
    while (1) {
        int n = read(ollama_fd, buf, sizeof(buf) - 1);
        if (n <= 0) break;
        buf[n] = 0;
        if (total + n < resp_size) {
            strcat(response, buf);
            total += n;
        }
        // Buscar fin de JSON
        if (strstr(buf, "}") != NULL) break;
    }

    return total;
}

int main(int argc, char *argv[]) {
    int dev_fd, ollama_fd;
    fd_set rfds;
    char buf[4096];
    char response[8192];

    printf("[iaosd] Iniciando IA Driver para JuliOS...\n");

    // Crear device
    create_device();

    // Conectar a Ollama (reintentar si falla)
    int retries = 5;
    while (retries-- > 0) {
        ollama_fd = connect_ollama();
        if (ollama_fd >= 0) break;
        sleep(2);
    }

    if (ollama_fd < 0) {
        fprintf(stderr, "[iaosd] No se pudo conectar a Ollama\n");
        return 1;
    }

    // Abrir device para lectura
    dev_fd = open(DEVICE_PATH, O_RDWR);
    if (dev_fd < 0) {
        perror("open device");
        return 1;
    }

    printf("[iaosd] Esperando comandos en %s\n", DEVICE_PATH);
    printf("[iaosd] Listo!\n");

    // Loop principal
    while (1) {
        FD_ZERO(&rfds);
        FD_SET(dev_fd, &rfds);

        struct timeval tv = { .tv_sec = 60, .tv_usec = 0 };
        int sel = select(dev_fd + 1, &rfds, NULL, NULL, &tv);

        if (sel > 0 && FD_ISSET(dev_fd, &rfds)) {
            int n = read(dev_fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = 0;
                printf("[iaosd] Recibido: %s\n", buf);

                // Enviar a Ollama
                int resp_len = forward_to_ollama(ollama_fd, buf, response, sizeof(response));
                if (resp_len > 0) {
                    // Escribir respuesta de vuelta
                    write(dev_fd, response, strlen(response));
                    printf("[iaosd] Respuesta enviada\n");
                }
            }
        }
    }

    close(dev_fd);
    close(ollama_fd);
    return 0;
}