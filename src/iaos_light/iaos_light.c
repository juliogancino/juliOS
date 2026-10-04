/*
 * iaos_light.c - Proceso de IA autónoma para JuliOS
 * 
 * Este proceso es el primer proceso en user-space que se ejecuta
 * al iniciar el sistema. Tiene prioridad máxima y se comunica con
 * el kernel via syscalls ia_send() e ia_recv().
 * 
 * Autor: Zuzebot (Asistente IA)
 * Fecha: 2026-03-17
 * Licencia: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

/* 
 * Tamaño máximo del buffer para mensajes 
 * Ajustar según necesidades del sistema
 */
#define MAX_MSG_SIZE 4096

/* 
 * Archivo de dispositivo para comunicación con el kernel
 * Esta es la interfaz principal - el kernel crea este dispositivo
 */
#define IAOS_DEVICE "/dev/iaos"

/* 
 * Flag para modo debug
 * Compilar con -DDEBUG para ver mensajes de debug
 */
#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "[IAOS_DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) 
#endif

/* 
 * Estructura para mensajes entre kernel y IA
 * Se comunica a través del dispositivo /dev/iaos
 */
typedef struct {
    char message[MAX_MSG_SIZE];  /* Mensaje propiamente dicho */
    int type;                    /* Tipo: 0=comando, 1=respuesta, 2=evento */
    int size;                     /* Tamaño del mensaje */
    int status;                   /* Estado de la operación */
} iaos_message_t;

/* 
 * Función: ia_send
 * Envía un mensaje desde user-space al kernel/IA
 * 
 * Parámetros:
 *   - msg: Puntero al mensaje a enviar
 *   - len: Longitud del mensaje
 * 
 * Retorna:
 *   - 0 si éxito
 *   - -1 si error (errno establecido)
 */
int ia_send(const char *msg, size_t len) {
    int fd;
    ssize_t bytes_written;
    
    if (msg == NULL || len == 0 || len >= MAX_MSG_SIZE) {
        errno = EINVAL;
        return -1;
    }
    
    /* Abrir dispositivo de comunicación */
    fd = open(IAOS_DEVICE, O_WRONLY);
    if (fd < 0) {
        /* Si el dispositivo no existe, usar método alternativo */
        /* Por ahora, retornamos error si no existe */
        DEBUG_PRINT("No se pudo abrir %s: %s", IAOS_DEVICE, strerror(errno));
        return -1;
    }
    
    /* Escribir mensaje al dispositivo */
    bytes_written = write(fd, msg, len);
    if (bytes_written < 0) {
        close(fd);
        return -1;
    }
    
    close(fd);
    DEBUG_PRINT("ia_send: enviado %zd bytes", bytes_written);
    return 0;
}

/* 
 * Función: ia_recv
 * Recibe una respuesta de la IA
 * 
 * Parámetros:
 *   - buffer: Donde almacenamos la respuesta
 *   - bufsize: Tamaño máximo del buffer
 * 
 * Retorna:
 *   - Bytes recibidos si éxito
 *   - -1 si error
 */
ssize_t ia_recv(char *buffer, size_t bufsize) {
    int fd;
    ssize_t bytes_read;
    
    if (buffer == NULL || bufsize == 0) {
        errno = EINVAL;
        return -1;
    }
    
    /* Abrir dispositivo en modo lectura */
    fd = open(IAOS_DEVICE, O_RDONLY);
    if (fd < 0) {
        DEBUG_PRINT("No se pudo abrir %s para lectura: %s", IAOS_DEVICE, strerror(errno));
        return -1;
    }
    
    /* Leer respuesta */
    bytes_read = read(fd, buffer, bufsize - 1);
    if (bytes_read < 0) {
        close(fd);
        return -1;
    }
    
    /* Null-terminar el buffer */
    buffer[bytes_read] = '\0';
    
    close(fd);
    DEBUG_PRINT("ia_recv: recibido %zd bytes", bytes_read);
    return bytes_read;
}

/* 
 * Función: ejecutar_comando
 * Ejecuta un comando y envía resultados a la IA
 * 
 * Esta función permite que la IA ejecute acciones
 * en el sistema de forma autónoma
 */
int ejecutar_comando(const char *cmd) {
    FILE *fp;
    char resultado[MAX_MSG_SIZE];
    
    if (cmd == NULL) return -1;
    
    /* Ejecutar comando usando popen */
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return -1;
    }
    
    /* Leer primeros bytes del resultado */
    if (fgets(resultado, sizeof(resultado), fp) != NULL) {
        /* Enviar resultado a la IA */
        ia_send(resultado, strlen(resultado));
    }
    
    pclose(fp);
    return 0;
}

/* 
 * Función: inicializar_iaos
 * Inicializa el proceso iaos_light
 * 
 * Configura:
 * - Prioridad máxima (nice value)
 * - Conexión con el kernel
 * - Inicializa el modelo de IA
 */
int inicializar_iaos(void) {
    int ret;
    
    /* Establecer prioridad máxima (solo root puede usar valores negativos) */
    /* Intentamosnice -20, si falla usamos el valor mínimo permitido */
    ret = nice(-20);
    if (ret == -1 && errno != 0) {
        /* Si no podemos reducir nice, intentamos con 0 (menor prioridad) */
        ret = nice(0);
        DEBUG_PRINT("No se pudo establecer prioridad máxima: %s", strerror(errno));
    }
    
    DEBUG_PRINT("iaos_light iniciado con nice value: %d", ret);
    
    /* Aquí se inicializaría Ollama o el modelo de IA */
    /* Por ahora, solo registramos que iniciamos */
    
    return 0;
}

/* 
 * Función: loop_principal
 * Bucle principal del proceso iaos_light
 * 
 * Espera mensajes del kernel y responde
 */
void loop_principal(void) {
    char buffer[MAX_MSG_SIZE];
    char respuesta[MAX_MSG_SIZE];
    ssize_t bytes_recibidos;
    
    printf("[iaos_light] Iniciando bucle principal...\n");
    
    while (1) {
        /* Limpiar buffer */
        memset(buffer, 0, MAX_MSG_SIZE);
        
        /* Esperar mensaje del kernel */
        bytes_recibidos = ia_recv(buffer, MAX_MSG_SIZE);
        
        if (bytes_recibidos > 0) {
            printf("[iaos_light] Mensaje recibido: %s\n", buffer);
            
            /* 
             * Aquí procesaríamos el mensaje y generaríamos respuesta
             * Por ahora, simulamos una respuesta
             */
            snprintf(respuesta, MAX_MSG_SIZE, "IAOS: Mensaje procesado '%s'", buffer);
            
            /* Enviar respuesta de vuelta */
            ia_send(respuesta, strlen(respuesta));
            
        } else if (bytes_recibidos < 0) {
            /* Error al recibir */
            DEBUG_PRINT("Error al recibir mensaje: %s", strerror(errno));
            sleep(1);  /* Esperar antes de reintentar */
        }
        
        /* Pequeña pausa para evitar consumo excesivo de CPU */
        usleep(100000);  /* 100ms */
    }
}

/* 
 * Función: modo_interactivo
 * Modo interactivo - permite comunicación directa con la IA
 */
void modo_interactivo(void) {
    char input[MAX_MSG_SIZE];
    
    printf("[iaos_light] Modo interactivo. Escribe 'salir' para terminar.\n");
    
    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        /* Eliminar salto de línea */
        input[strcspn(input, "\n")] = 0;
        
        /* Verificar comando de salida */
        if (strcmp(input, "salir") == 0 || strcmp(input, "exit") == 0) {
            break;
        }
        
        /* Enviar mensaje a la IA */
        if (ia_send(input, strlen(input)) == 0) {
            /* Obtener respuesta */
            char respuesta[MAX_MSG_SIZE];
            ssize_t bytes = ia_recv(respuesta, MAX_MSG_SIZE);
            if (bytes > 0) {
                printf("IA: %s\n", respuesta);
            }
        }
    }
}

/* 
 * Función: mostrar_ayuda
 * Muestra las opciones de uso del programa
 */
void mostrar_ayuda(const char *prog_name) {
    printf("Uso: %s [opciones]\n", prog_name);
    printf("\nOpciones:\n");
    printf("  -d, --daemon    Ejecutar como daemon (bucle infinito)\n");
    printf("  -i, --interactive  Modo interactivo\n");
    printf("  -h, --help      Mostrar esta ayuda\n");
    printf("  -v, --version   Mostrar versión\n");
    printf("\nDescripción:\n");
    printf("  iaos_light es el proceso de IA autónoma para JuliOS.\n");
    printf("  Se comunica con el kernel via syscalls ia_send/ia_recv.\n");
}

#define VERSION "1.0.0"

/* 
 * Función principal
 * Punto de entrada del programa iaos_light
 */
int main(int argc, char *argv[]) {
    int modo_daemon = 0;
    int modo_inter = 0;
    int opt;
    
    /* Procesar argumentos de línea de comandos */
    while ((opt = getopt(argc, argv, "divh")) != -1) {
        switch (opt) {
            case 'd':
                modo_daemon = 1;
                break;
            case 'i':
                modo_inter = 1;
                break;
            case 'h':
                mostrar_ayuda(argv[0]);
                return 0;
            case 'v':
                printf("iaos_light version %s\n", VERSION);
                return 0;
            default:
                mostrar_ayuda(argv[0]);
                return 1;
        }
    }
    
    printf("=== JuliOS IAOS Light v%s ===\n", VERSION);
    printf("Proceso de IA autónoma para JuliOS\n");
    printf("Autor: Zuzebot\n\n");
    
    /* Inicializar */
    if (inicializar_iaos() != 0) {
        fprintf(stderr, "Error al inicializar iaos_light\n");
        return 1;
    }
    
    /* Ejecutar según modo */
    if (modo_inter) {
        modo_interactivo();
    } else if (modo_daemon) {
        loop_principal();
    } else {
        /* Por defecto, ejecutar en modo daemon */
        loop_principal();
    }
    
    return 0;
}