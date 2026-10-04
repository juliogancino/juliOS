/*
 * iaos_user.h - Header para programas de usuario
 * 
 * Proporciona funciones wrapper para syscalls ia_send/ia_recv
 * Acceso al dispositivo /dev/iaos
 */

#ifndef _IAOS_USER_H
#define _IAOS_USER_H

#include <stddef.h>
#include <stdint.h>

/* 
 * Constantes 
 */
#define IAOS_DEVICE_PATH  "/dev/iaos"
#define IAOS_BUFFER_SIZE  4096

/*
 * Tipos de mensajes
 */
#define IAOS_TYPE_CMD     0   /* Comando del usuario */
#define IAOS_TYPE_RESP    1   /* Respuesta de la IA */
#define IAOS_TYPE_EVENT   2   /* Evento del sistema */

/*
 * Códigos de retorno
 */
#define IAOS_SUCCESS      0
#define IAOS_ERROR       -1
#define IAOS_EOPEN       -2   /* Error al abrir dispositivo */
#define IAOS_EWRITE      -3   /* Error al escribir */
#define IAOS_EREAD       -4   /* Error al leer */
#define IAOS_ETIMEOUT    -5   /* Timeout */

/*
 * Estructura de mensaje
 */
typedef struct {
    char     data[IAOS_BUFFER_SIZE];  /* Datos del mensaje */
    uint32_t type;                     /* Tipo de mensaje */
    uint32_t size;                     /* Tamaño de datos */
    int32_t  status;                   /* Estado de la operación */
} iaos_msg_t;

/*
 * Funciones de alto nivel
 */

/* 
 * iaos_init - Inicializar conexión con el kernel
 * Retorna: 0 si éxito, código de error si falla
 */
int iaos_init(void);

/* 
 * iaos_send - Enviar mensaje a la IA
 * 
 * Parámetros:
 *   - message: Cadena con el mensaje
 *   - size:    Longitud del mensaje
 * 
 * Retorna: Bytes enviados o código de error
 */
ssize_t iaos_send(const char *message, size_t size);

/* 
 * iaos_recv - Recibir respuesta de la IA
 * 
 * Parámetros:
 *   - buffer: Donde almacenar la respuesta
 *   - size:   Tamaño máximo del buffer
 * 
 * Retorna: Bytes recibidos o código de error
 */
ssize_t iaos_recv(char *buffer, size_t size);

/*
 * iaos_close - Cerrar conexión
 */
void iaos_close(void);

/*
 * Funciones de bajo nivel (syscalls)
 */

/*
 * iaos_syscall_send - Syscall directa (si está implementada)
 */
long iaos_syscall_send(const char *msg, size_t len);

/*
 * iaos_syscall_recv - Syscall directa (si está implementada)
 */
long iaos_syscall_recv(char *buf, size_t len);

/* 
 * Funciones de utilidad 
 */

/* 
 * iaos_ping - Verificar si el kernel responde
 * Retorna: 1 si el kernel responde, 0 si no
 */
int iaos_ping(void);

/*
 * iaos_status - Obtener estado del sistema IA
 * Retorna: String con el estado (caller debe liberar)
 */
char* iaos_status(void);

/*
 * Versión del módulo kernel
 */
#define IAOS_VERSION "1.0.0"

#endif /* _IAOS_USER_H */