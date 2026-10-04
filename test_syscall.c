#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>

#define SYS_ia_send 451
#define SYS_ia_recv 452

int main(int argc, char *argv[]) {
    printf("=== Test Syscalls JuliOS ===\n");
    
    // Test ia_send
    const char *prompt = "Hello";
    long result = syscall(SYS_ia_send, prompt, strlen(prompt), 0, 256);
    printf("ia_send(451) returned: %ld, errno: %d (%s)\n", result, errno, strerror(errno));
    
    // Test ia_recv
    char buffer[256];
    memset(buffer, 0, 256);
    result = syscall(SYS_ia_recv, buffer, 256);
    printf("ia_recv(452) returned: %ld, errno: %d (%s)\n", result, errno, strerror(errno));
    
    return 0;
}
