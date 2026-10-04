#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>

#define SYS_IA_SEND 451
#define RESPONSE_SIZE 8192

static double elapsed_ms(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

int main(int argc, char *argv[])
{
    const char *prompt = "Responde exactamente JULIOS_OK";
    int repetitions = 30;
    int i;

    if (argc >= 2) {
        repetitions = atoi(argv[1]);

        if (repetitions <= 0 || repetitions > 10000) {
            fprintf(stderr, "Numero de repeticiones invalido\n");
            return 1;
        }
    }

    printf("run,return,errno,latency_ms,response\n");
    fflush(stdout);

    for (i = 1; i <= repetitions; i++) {
        char response[RESPONSE_SIZE];
        struct timespec start, end;
        long ret;
        int saved_errno;
        double latency;

        memset(response, 0, sizeof(response));
        errno = 0;

        if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
            perror("clock_gettime start");
            return 1;
        }

        ret = syscall(SYS_IA_SEND,
                      prompt,
                      (int)strlen(prompt),
                      response,
                      sizeof(response));

        saved_errno = errno;

        if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
            perror("clock_gettime end");
            return 1;
        }

        latency = elapsed_ms(start, end);

        /*
         * Evitar que una respuesta inesperada rompa
         * la estructura CSV.
         */
        for (char *p = response; *p; p++) {
            if (*p == ',' || *p == '\n' || *p == '\r')
                *p = ' ';
        }

        printf("%d,%ld,%d,%.3f,%s\n",
               i,
               ret,
               saved_errno,
               latency,
               response);

        fflush(stdout);
    }

    return 0;
}
