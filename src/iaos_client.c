/*
 * iaos_client.c - Cliente de prueba para comunicación con la IA
 * 
 * Este programa permite enviar mensajes a la IA y recibir respuestas
 * a través del dispositivo /dev/iaos
 * 
 * Uso: ./iaos_client <mensaje>
 *       ./iaos_client -f <archivo>
 *       ./iaos_client -i (modo interactivo)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEVICE_PATH "/dev/iaos"
#define BUFFER_SIZE 4096

/* Enviar mensaje al dispositivo */
int send_message(const char *msg) {
    int fd;
    ssize_t bytes_written;
    
    fd = open(DEVICE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Error al abrir el dispositivo");
        return -1;
    }
    
    bytes_written = write(fd, msg, strlen(msg));
    if (bytes_written < 0) {
        perror("Error al escribir");
        close(fd);
        return -1;
    }
    
    printf("Enviado: %zd bytes\n", bytes_written);
    close(fd);
    return 0;
}

/* Recibir respuesta del dispositivo */
int receive_message(char *buffer, size_t size) {
    int fd;
    ssize_t bytes_read;
    
    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("Error al abrir el dispositivo para lectura");
        return -1;
    }
    
    bytes_read = read(fd, buffer, size - 1);
    if (bytes_read < 0) {
        perror("Error al leer");
        close(fd);
        return -1;
    }
    
    buffer[bytes_read] = '\0';
    printf("Recibido: %zd bytes\n", bytes_read);
    close(fd);
    return bytes_read;
}

/* Modo interactivo */
void interactive_mode(void) {
    char input[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    
    printf("=== Modo Interactivo IAOS ===\n");
    printf("Escribe 'salir' para terminar\n\n");
    
    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        /* Eliminar salto de línea */
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "salir") == 0 || strcmp(input, "exit") == 0) {
            break;
        }
        
        if (strlen(input) == 0) {
            continue;
        }
        
        /* Enviar mensaje */
        if (send_message(input) == 0) {
            /* Obtener respuesta */
            if (receive_message(response, sizeof(response)) > 0) {
                printf("IA: %s\n\n", response);
            }
        }
    }
}

/* Leer mensaje desde archivo */
int send_from_file(const char *filename) {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error al abrir archivo");
        return -1;
    }
    
    bytes_read = fread(buffer, 1, BUFFER_SIZE - 1, fp);
    buffer[bytes_read] = '\0';
    fclose(fp);
    
    printf("Enviando contenido de %s (%zu bytes)\n", filename, bytes_read);
    
    if (send_message(buffer) == 0) {
        char response[BUFFER_SIZE];
        if (receive_message(response, sizeof(response)) > 0) {
            printf("Respuesta: %s\n", response);
        }
    }
    
    return 0;
}

/* Mostrar ayuda */
void show_help(const char *prog) {
    printf("Uso: %s [opciones] [mensaje]\n", prog);
    printf("\nOpciones:\n");
    printf("  -i, --interactive  Modo interactivo\n");
    printf("  -f, --file <archivo>  Enviar contenido de archivo\n");
    printf("  -h, --help        Mostrar esta ayuda\n");
    printf("\nEjemplos:\n");
    printf("  %s \"Hola IA\"\n", prog);
    printf("  %s -i\n", prog);
    printf("  %s -f mensaje.txt\n", prog);
}

int main(int argc, char *argv[]) {
    int opt;
    int interactive = 0;
    char *filename = NULL;
    char *message = NULL;
    
    /* Parsear argumentos */
    while ((opt = getopt(argc, argv, "if:h")) != -1) {
        switch (opt) {
            case 'i':
                interactive = 1;
                break;
            case 'f':
                filename = optarg;
                break;
            case 'h':
                show_help(argv[0]);
                return 0;
            default:
                show_help(argv[0]);
                return 1;
        }
    }
    
    /* Obtener mensaje pos-argumento */
    if (optind < argc) {
        message = argv[optind];
    }
    
    /* Verificar dispositivo */
    if (access(DEVICE_PATH, F_OK) != 0) {
        fprintf(stderr, "Error: %s no existe.\n", DEVICE_PATH);
        fprintf(stderr, "Asegúrate de que el módulo kernel esté cargado.\n");
        return 1;
    }
    
    /* Ejecutar según modo */
    if (interactive) {
        interactive_mode();
    } else if (filename) {
        send_from_file(filename);
    } else if (message) {
        char response[BUFFER_SIZE];
        
        if (send_message(message) == 0) {
            if (receive_message(response, sizeof(response)) > 0) {
                printf("IA: %s\n", response);
            }
        }
    } else {
        /* Sin argumentos, mostrar ayuda */
        show_help(argv[0]);
        return 1;
    }
    
    return 0;
}