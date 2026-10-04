/*
 * iaos_kernel.c - Módulo del kernel de JuliOS para comunicación con IA
 * 
 * Este módulo crea el dispositivo /dev/iaos que permite la comunicación
 * entre el proceso iaos_light (user-space) y el kernel.
 * 
 * Proporciona:
 * - ia_send(): Enviar mensaje desde kernel/userspace a la IA
 * - ia_recv(): Recibir respuesta de la IA
 * 
 * Autor: Zuzebot
 * Fecha: 2026-03-17
 * Licencia: GPL v2
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

/* 
 * Configuración del módulo
 */
#define DEVICE_NAME "iaos"        /* Nombre del dispositivo */
#define CLASS_NAME  "iaos_class"   /* Nombre de la clase */
#define BUFFER_SIZE 4096           /* Tamaño del buffer de mensajes */

/* 
 * Información del módulo
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zuzebot");
MODULE_DESCRIPTION("Modulo de comunicacion kernel-IA para JuliOS");
MODULE_VERSION("1.0");

/*
 * Estructura para almacenar datos del dispositivo
 */
struct iaos_dev {
    dev_t           dev_num;      /* Número de dispositivo */
    struct cdev     cdev;         /* Estructura del character device */
    struct class    *class;       /* Clase del dispositivo */
    struct device   *device;      /* Puntero al dispositivo */
    char            *rx_buffer;   /* Buffer para mensaje recibido */
    char            *tx_buffer;   /* Buffer para mensaje a enviar */
    size_t          rx_size;      /* Tamaño del mensaje recibido */
    size_t          tx_size;      /* Tamaño del mensaje a enviar */
    struct mutex    lock;         /* Mutex para sincronización */
    wait_queue_head_t read_queue; /* Cola de lectura */
    int             has_data;     /* Flag: hay datos disponibles */
};

/* Variables globales */
static struct iaos_dev *iaos_device;
static int major_number;
static struct proc_dir_entry *proc_entry;

/* Declaraciones de funciones */
static int     iaos_open(struct inode *inode, struct file *file);
static int     iaos_release(struct inode *inode, struct file *file);
static ssize_t iaos_read(struct file *file, char __user *buf, 
                         size_t count, loff_t *offset);
static ssize_t iaos_write(struct file *file, const char __user *buf, 
                          size_t count, loff_t *offset);

/* 
 * Operaciones del dispositivo
 * Define qué funciones se ejecutan cuando se abre, lee, escribe, etc.
 */
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = iaos_open,
    .release = iaos_release,
    .read    = iaos_read,
    .write   = iaos_write,
};

/*
 * Función: iaos_open
 * Se ejecuta cuando se abre el dispositivo
 * 
 * Parámetros:
 *   - inode: Puntero al inodo del dispositivo
 *   - file: Puntero al archivo abierto
 * 
 * Retorna: 0 si éxito, código de error si falla
 */
static int iaos_open(struct inode *inode, struct file *file) {
    struct iaos_dev *dev;
    
    /* Obtener referencia al dispositivo desde el inodo */
    dev = container_of(inode->i_cdev, struct iaos_dev, cdev);
    file->private_data = dev;
    
    pr_info("[IAOS] Dispositivo abierto\n");
    return 0;
}

/*
 * Función: iaos_release
 * Se ejecuta cuando se cierra el dispositivo
 */
static int iaos_release(struct inode *inode, struct file *file) {
    pr_info("[IAOS] Dispositivo cerrado\n");
    return 0;
}

/*
 * Función: iaos_read
 * Lee datos del buffer de la IA (enviados por el kernel)
 * 
 * Esta función implementa ia_recv() a nivel de kernel
 */
static ssize_t iaos_read(struct file *file, char __user *buf, 
                         size_t count, loff_t *offset) {
    struct iaos_dev *dev = file->private_data;
    ssize_t bytes_to_read;
    
    /* Verificar que haya datos disponibles */
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;
    
    /* Si no hay datos, esperar */
    if (!dev->has_data) {
        mutex_unlock(&dev->lock);
        return 0;
    }
    
    /* Calcular bytes a leer */
    bytes_to_read = min(count, dev->rx_size);
    
    /* Copiar datos al espacio de usuario */
    if (copy_to_user(buf, dev->rx_buffer, bytes_to_read)) {
        mutex_unlock(&dev->lock);
        return -EFAULT;
    }
    
    /* Limpiar flag de datos */
    dev->has_data = 0;
    dev->rx_size = 0;
    
    mutex_unlock(&dev->lock);
    
    pr_info("[IAOS] Enviados %zd bytes a user-space\n", bytes_to_read);
    return bytes_to_read;
}

/*
 * Función: iaos_write
 * Escribe datos desde user-space al buffer de la IA
 * 
 * Esta función implementa ia_send() a nivel de kernel
 */
static ssize_t iaos_write(struct file *file, const char __user *buf, 
                          size_t count, loff_t *offset) {
    struct iaos_dev *dev = file->private_data;
    ssize_t bytes_written;
    
    /* Verificar tamaño del mensaje */
    if (count > BUFFER_SIZE) {
        pr_err("[IAOS] Mensaje demasiado grande: %zu bytes\n", count);
        return -EINVAL;
    }
    
    /* Bloquear mutex para acceso seguro */
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;
    
    /* Copiar datos desde espacio de usuario */
    bytes_written = count;
    if (copy_from_user(dev->tx_buffer, buf, bytes_written)) {
        mutex_unlock(&dev->lock);
        return -EFAULT;
    }
    
    /* Asegurar null-terminación */
    dev->tx_buffer[bytes_written] = '\0';
    dev->tx_size = bytes_written;
    
    /* 
     * Aquí es donde normalmente llamaríamos a la IA (Ollama)
     * Por ahora, simplemente copiamos al buffer de lectura
     * y simulamos una respuesta
     */
    
    /* Copiar al buffer de lectura (respuesta simulada) */
    snprintf(dev->rx_buffer, BUFFER_SIZE, 
             "[IAOS Kernel] Mensaje recibido: %s", dev->tx_buffer);
    dev->rx_size = strlen(dev->rx_buffer);
    dev->has_data = 1;
    
    /* Despertar procesos esperando lectura */
    wake_up_interruptible(&dev->read_queue);
    
    mutex_unlock(&dev->lock);
    
    pr_info("[IAOS] Recibidos %zd bytes desde user-space\n", bytes_written);
    return bytes_written;
}

/*
 * Función: iaos_proc_read
 * Lee información del módulo vía /proc/iaos
 */
static ssize_t iaos_proc_read(struct file *file, char __user *buf,
                               size_t count, loff_t *offset) {
    int len;
    char info[256];
    
    len = snprintf(info, sizeof(info),
                   "=== JuliOS IAOS Kernel Module ===\n"
                   "Estado: Activo\n"
                   "Dispositivo: /dev/%s\n"
                   "Buffer size: %d bytes\n"
                   "Version: %s\n",
                   DEVICE_NAME, BUFFER_SIZE, "1.0.0");
    
    if (*offset >= len)
        return 0;
    
    if (count > len - *offset)
        count = len - *offset;
    
    if (copy_to_user(buf, info + *offset, count))
        return -EFAULT;
    
    *offset += count;
    return count;
}

/* 
 * Estructura para operaciones de /proc 
 */
static const struct proc_ops iaos_proc_ops = {
    .proc_read = iaos_proc_read,
};

/*
 * Función: iaos_init
 * Inicializa el módulo del kernel
 * 
 * Retorna: 0 si éxito, código de error si falla
 */
static int __init iaos_init(void) {
    int ret;
    
    pr_info("[IAOS] Inicializando modulo kernel...\n");
    
    /* Reservar número de dispositivo */
    ret = alloc_chrdev_region(&iaos_device->dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("[IAOS] Error al reservarr numero de dispositivo: %d\n", ret);
        goto fail_devnum;
    }
    major_number = MAJOR(iaos_device->dev_num);
    pr_info("[IAOS] Major number: %d\n", major_number);
    
    /* Inicializar cdev */
    cdev_init(&iaos_device->cdev, &fops);
    iaos_device->cdev.owner = THIS_MODULE;
    
    /* Agregar cdev al sistema */
    ret = cdev_add(&iaos_device->cdev, iaos_device->dev_num, 1);
    if (ret < 0) {
        pr_err("[IAOS] Error al agregar cdev: %d\n", ret);
        goto fail_cdev;
    }
    
    /* Crear clase de dispositivo */
    iaos_device->class = class_create(CLASS_NAME);
    if (IS_ERR(iaos_device->class)) {
        pr_err("[IAOS] Error al crear clase\n");
        ret = PTR_ERR(iaos_device->class);
        goto fail_class;
    }
    
    /* Crear dispositivo */
    iaos_device->device = device_create(iaos_device->class, NULL,
                                          iaos_device->dev_num, NULL,
                                          DEVICE_NAME);
    if (IS_ERR(iaos_device->device)) {
        pr_err("[IAOS] Error al crear dispositivo\n");
        ret = PTR_ERR(iaos_device->device);
        goto fail_device;
    }
    
    /* Crear entrada en /proc */
    proc_entry = proc_create("iaos", 0444, NULL, &iaos_proc_ops);
    if (!proc_entry) {
        pr_warn("[IAOS] No se pudo crear /proc/iaos\n");
        /* No es fatal, continuamos */
    }
    
    pr_info("[IAOS] Modulo cargado exitosamente\n");
    pr_info("[IAOS] Dispositivo disponible en /dev/%s\n", DEVICE_NAME);
    
    return 0;

fail_device:
    class_destroy(iaos_device->class);
fail_class:
    cdev_del(&iaos_device->cdev);
fail_cdev:
    unregister_chrdev_region(iaos_device->dev_num, 1);
fail_devnum:
    kfree(iaos_device);
    return ret;
}

/*
 * Función: iaos_exit
 * Limpia el módulo al descargar
 */
static void __exit iaos_exit(void) {
    pr_info("[IAOS] Descargando modulo...\n");
    
    /* Remover /proc */
    if (proc_entry)
        proc_remove(proc_entry);
    
    /* Destruir dispositivo */
    if (iaos_device->class)
        device_destroy(iaos_device->class, iaos_device->dev_num);
    
    /* Destruir clase */
    if (iaos_device->class)
        class_destroy(iaos_device->class);
    
    /* Remover cdev */
    cdev_del(&iaos_device->cdev);
    
    /* Liberar número de dispositivo */
    unregister_chrdev_region(iaos_device->dev_num, 1);
    
    /* Liberar memoria */
    if (iaos_device->rx_buffer)
        kfree(iaos_device->rx_buffer);
    if (iaos_device->tx_buffer)
        kfree(iaos_device->tx_buffer);
    kfree(iaos_device);
    
    pr_info("[IAOS] Modulo descargado\n");
}

/* 
 * Macros para registro de funciones de inicialización/limpieza 
 */
module_init(iaos_init);
module_exit(iaos_exit);