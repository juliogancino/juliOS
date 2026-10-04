/*
 * IA Syscalls for JuliOS
 * Syscalls ia_send e ia_recv para comunicación con Ollama
 * Compatible con Linux kernel 6.1
 */

#include <linux/linkage.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/in.h>
#include <net/sock.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/in.h>
#include <net/ip.h>
#include <linux/inet.h>
#include <linux/uio.h>

/* Configuración de Ollama */
#define OLLAMA_PORT 11434
#define OLLAMA_HOST "10.0.2.2"  /* IP del host desde QEMU */

/* Buffer máximo */
#define IA_MAX_BUF 8192

/*
 * sys_ia_send - Enviar prompt a IA y obtener respuesta
 */
SYSCALL_DEFINE4(ia_send, const char __user *, prompt, int, prompt_len,
		char __user *, response, int, response_len)
{
	struct socket *sock = NULL;
	struct sockaddr_in server;
	struct msghdr msg, rmsg;
	struct kvec iov, riov;
	char *kbuf = NULL;
	char *resp_buf = NULL;
	int ret = 0;
	int bytes;
	int req_len;
	int total;
	int copy_len;
	char *json_start;
	char *json_end;

	/* Validar parámetros */
	if (!prompt || prompt_len <= 0 || prompt_len > IA_MAX_BUF)
		return -EINVAL;
	if (!response || response_len <= 0)
		return -EINVAL;

	/* Asignar memoria kernel */
	kbuf = kzalloc(IA_MAX_BUF, GFP_KERNEL);
	resp_buf = kzalloc(IA_MAX_BUF, GFP_KERNEL);
	if (!kbuf || !resp_buf) {
		ret = -ENOMEM;
		goto out;
	}

	/* Copiar prompt desde user space */
	kbuf[prompt_len] = '\0';
	if (copy_from_user(kbuf, prompt, prompt_len)) {
		ret = -EFAULT;
		goto out;
	}

	/* Crear socket */
	ret = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
	if (ret < 0) {
		goto out;
	}

	/* Configurar dirección del server */
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(OLLAMA_PORT);
	server.sin_addr.s_addr = in_aton(OLLAMA_HOST);

	/* Conectar */
        pr_info("JULIOS_TRACE:antes de kernel_connect\n");
	ret = kernel_connect(sock, (struct sockaddr *)&server, sizeof(server), 0);
        pr_info("JULIOS_TRACE: kernel_connect retorno=%d\n",ret);
	if (ret < 0) {
		goto out;
	}

	/* Construir request JSON simple */
	req_len = snprintf(kbuf, IA_MAX_BUF,
		"{\"model\":\"minimax-m2.5:cloud\",\"prompt\":\"%.4096s\",\"stream\":false}",
		kbuf);

	/* Enviar request usando msghdr con iov_iter */
	memset(&msg, 0, sizeof(msg));
	iov.iov_base = kbuf;
	iov.iov_len = req_len;
        pr_info("JULIOS_TRACE: antes de kernel_sendmsg req_len=%d\n",req_len);
	bytes = kernel_sendmsg(sock, &msg, &iov, 1, req_len);
        pr_info("JULIOS_TRACE: kernel_sendmsg retorno=%d\n",bytes);
	if (bytes < 0) {
		ret = bytes;
		goto out;
	}

	/* Leer respuesta */
	total = 0;
	while (total < IA_MAX_BUF - 1) {
		memset(&rmsg, 0, sizeof(rmsg));
		riov.iov_base = resp_buf + total;
		riov.iov_len = IA_MAX_BUF - total - 1;
		pr_info("JULIOS_TRACE: antes de kernel_recvmsg total=%d\n",total);
                bytes = kernel_recvmsg(sock, &rmsg, &riov, 1,
				       riov.iov_len, 0);
                pr_info("JULIOS_TRACE: kernel_recvmsg retorna=%d\n",bytes);
		if (bytes <= 0) break;
		total += bytes;
		/* Buscar fin de JSON */
		if (total > 10 && resp_buf[total-1] == '}') break;
	}
	resp_buf[total] = '\0';

	/* Extraer solo el campo "response" del JSON */
	json_start = strstr(resp_buf, "\"response\":\"");
	if (json_start) {
		json_start += 11;  /* Saltar "\"response\":\"" */
		json_end = strstr(json_start, "\"");
		if (json_end) *json_end = '\0';
		
		/* Copiar respuesta a user space */
		copy_len = min_t(int, strlen(json_start), response_len - 1);
		if (copy_to_user(response, json_start, copy_len)) {
			ret = -EFAULT;
			goto out;
		}
		response[copy_len] = '\0';
		ret = copy_len;
	} else {
		/* Si no es JSON válido, devolver lo que hay */
		copy_len = min_t(int, total, response_len - 1);
		if (copy_to_user(response, resp_buf, copy_len)) {
			ret = -EFAULT;
			goto out;
		}
		response[copy_len] = '\0';
		ret = copy_len;
	}

out:
	if (sock) sock_release(sock);
	kfree(kbuf);
	kfree(resp_buf);
	return ret;
}

/*
 * sys_ia_recv - Recibir respuesta de IA (reservado para uso futuro)
 */
SYSCALL_DEFINE2(ia_recv, char __user *, buffer, int, len)
{
	if (!buffer || len <= 0)
		return -EINVAL;
	
	return -ENOSYS;
}
