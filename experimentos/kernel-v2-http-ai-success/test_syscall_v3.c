#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>

#define SYS_ia_send 451

int main(void)
{
    const char *prompt = "Responde exactamente JULIOS_OK";
    char response[1024] = {0};
    long ret;

    printf("=== JulioOS ia_send validation v3 ===\n");
    printf("Prompt: %s\n\n", prompt);

    errno = 0;

    ret = syscall(SYS_ia_send,
                  prompt,
                  strlen(prompt),
                  response,
                  sizeof(response));

    printf("syscall 451 (ia_send)\n");
    printf("return   = %ld\n", ret);
    printf("errno    = %d\n", errno);
    printf("error    = %s\n",
           errno ? strerror(errno) : "Success");

    if (ret >= 0)
        printf("response = %s\n", response);

    return 0;
}
