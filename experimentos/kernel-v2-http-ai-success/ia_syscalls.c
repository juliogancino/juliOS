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
	char *prompt_buf = NULL;
	char *json_buf = NULL;
	char *request_buf = NULL;
	char *resp_buf = NULL;
	char *body;
	char *json_start;
	char *json_end;
	int json_len;
	int req_len;
	int bytes;
	int total = 0;
	int copy_len;
	int ret = 0;

	if (!prompt || prompt_len <= 0 || prompt_len > 4096)
		return -EINVAL;

	if (!response || response_len <= 1)
		return -EINVAL;

	prompt_buf = kzalloc(4097, GFP_KERNEL);
	json_buf = kzalloc(IA_MAX_BUF, GFP_KERNEL);
	request_buf = kzalloc(IA_MAX_BUF, GFP_KERNEL);
	resp_buf = kzalloc(IA_MAX_BUF, GFP_KERNEL);

	if (!prompt_buf || !json_buf || !request_buf || !resp_buf) {
		ret = -ENOMEM;
		goto out;
	}

	if (copy_from_user(prompt_buf, prompt, prompt_len)) {
		ret = -EFAULT;
		goto out;
	}

	prompt_buf[prompt_len] = '\0';

	/*
	 * V2 experimental:
	 * Construir JSON en un buffer separado.
	 */
	json_len = snprintf(json_buf, IA_MAX_BUF,
		"{\"model\":\"qwen2.5-coder:1.5b\","
		"\"prompt\":\"%s\","
		"\"stream\":false}",
		prompt_buf);

	if (json_len <= 0 || json_len >= IA_MAX_BUF) {
		ret = -E2BIG;
		goto out;
	}

	/*
	 * Construir una petición HTTP válida para Ollama.
	 */
	req_len = snprintf(request_buf, IA_MAX_BUF,
		"POST /api/generate HTTP/1.1\r\n"
		"Host: " OLLAMA_HOST ":%d\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: %d\r\n"
		"Connection: close\r\n"
		"\r\n"
		"%s",
		OLLAMA_PORT, json_len, json_buf);

	if (req_len <= 0 || req_len >= IA_MAX_BUF) {
		ret = -E2BIG;
		goto out;
	}

	ret = sock_create_kern(&init_net, AF_INET, SOCK_STREAM,
			       IPPROTO_TCP, &sock);
	if (ret < 0)
		goto out;

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(OLLAMA_PORT);
	server.sin_addr.s_addr = in_aton(OLLAMA_HOST);

	pr_info("JULIOS_TRACE: antes de kernel_connect\n");

	ret = kernel_connect(sock, (struct sockaddr *)&server,
			     sizeof(server), 0);

	pr_info("JULIOS_TRACE: kernel_connect retorno=%d\n", ret);

	if (ret < 0)
		goto out;

	memset(&msg, 0, sizeof(msg));
	iov.iov_base = request_buf;
	iov.iov_len = req_len;

	pr_info("JULIOS_TRACE: HTTP request bytes=%d\n", req_len);

	bytes = kernel_sendmsg(sock, &msg, &iov, 1, req_len);

	pr_info("JULIOS_TRACE: kernel_sendmsg retorno=%d\n", bytes);

	if (bytes < 0) {
		ret = bytes;
		goto out;
	}

	if (bytes != req_len) {
		ret = -EIO;
		goto out;
	}

	while (total < IA_MAX_BUF - 1) {
		memset(&rmsg, 0, sizeof(rmsg));

		riov.iov_base = resp_buf + total;
		riov.iov_len = IA_MAX_BUF - total - 1;

		pr_info("JULIOS_TRACE: antes de kernel_recvmsg total=%d\n",
			total);

		bytes = kernel_recvmsg(sock, &rmsg, &riov, 1,
				       riov.iov_len, 0);

		pr_info("JULIOS_TRACE: kernel_recvmsg retorno=%d\n", bytes);

		if (bytes < 0) {
			ret = bytes;
			goto out;
		}

		if (bytes == 0)
			break;

		total += bytes;
	}

	resp_buf[total] = '\0';

	pr_info("JULIOS_TRACE: HTTP response total=%d\n", total);

	/*
	 * Separar cabeceras HTTP del cuerpo.
	 */
	body = strstr(resp_buf, "\r\n\r\n");
	if (!body) {
		ret = -EPROTO;
		goto out;
	}

	body += 4;

	/*
	 * Extraer el campo response del JSON devuelto por Ollama.
	 * Es un parser mínimo para esta fase experimental.
	 */
	json_start = strstr(body, "\"response\":\"");

	if (!json_start) {
		ret = -EPROTO;
		goto out;
	}

	json_start += strlen("\"response\":\"");
	json_end = strchr(json_start, '"');

	if (!json_end) {
		ret = -EPROTO;
		goto out;
	}

	copy_len = min_t(int, json_end - json_start, response_len - 1);

	if (copy_to_user(response, json_start, copy_len)) {
		ret = -EFAULT;
		goto out;
	}

	/*
	 * El terminador también debe copiarse de forma segura
	 * hacia memoria de usuario.
	 */
	{
		char nul = '\0';

		if (copy_to_user(response + copy_len, &nul, 1)) {
			ret = -EFAULT;
			goto out;
		}
	}

	ret = copy_len;

	pr_info("JULIOS_TRACE: respuesta IA copiada bytes=%d\n", copy_len);

out:
	if (sock)
		sock_release(sock);

	kfree(prompt_buf);
	kfree(json_buf);
	kfree(request_buf);
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
