#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>

#define SYS_ia_send 451
#define SYS_ia_recv 452

int main(void)
{
    long ret;
    char buffer[256] = {0};
    const char *prompt = "Prueba JulioOS";

    printf("=== JulioOS syscall validation v2 ===\n\n");

    errno = 0;
    ret = syscall(SYS_ia_send,
                  prompt,
                  strlen(prompt),
                  NULL,
                  sizeof(buffer));

    printf("syscall 451 (ia_send)\n");
    printf("return = %ld\n", ret);
    printf("errno  = %d\n", errno);
    printf("error  = %s\n\n",
           errno ? strerror(errno) : "Success");

    errno = 0;
    ret = syscall(SYS_ia_recv,
                  buffer,
                  sizeof(buffer));

    printf("syscall 452 (ia_recv)\n");
    printf("return = %ld\n", ret);
    printf("errno  = %d\n", errno);
    printf("error  = %s\n",
           errno ? strerror(errno) : "Success");

    return 0;
}
