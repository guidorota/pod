#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>

#define NEW_ROOT "/home/sidewinder/test/core"

static int child(void *arg) {
    struct utsname uts;

    if (sethostname(arg, strlen(arg)) == -1) {
        perror("sethostname child");
        exit(1);
    }

    if (chroot(NEW_ROOT) == -1) {
        perror("chroot");
        exit(1);
    }
    if (chdir("/") == -1) {
        perror("chdir");
        exit(1);
    }

    if (uname(&uts) == -1) {
        perror("uname");
        exit(1);
    }
    printf("uts.nodename in child: %s\n", uts.nodename);

    if (execl("/bin/bash", "bash", NULL) == -1) {
        perror("execv");
        exit(1);
    }

    return 0;
}

#define STACK_SIZE (1024 * 1024)

int main(int argc, char **argv) {
    char *stack;
    char *stack_top;
    pid_t pid;
    struct utsname uts;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <child-hostname>\n", argv[0]);
        exit(0);
    }

    stack = malloc(STACK_SIZE);
    if (stack == NULL) {
        perror("malloc");
        exit(1);
    }
    stack_top = stack + STACK_SIZE;

    pid = clone(child, stack_top, CLONE_NEWNET | CLONE_NEWNS | CLONE_NEWUTS |
            CLONE_NEWPID | CLONE_NEWIPC | SIGCHLD, argv[1]);
    if (pid == -1) {
        perror("clone");
        exit(1);
    }

    if (uname(&uts) == -1) {
        perror("uname");
        exit(1);
    }
    printf("uts.nodename in parent: %s\n", uts.nodename);

    if (waitpid(pid, NULL, 0) == -1) {
        perror("waitpid");
        exit(1);
    }
    printf("child has terminated\n");

    exit(0);
}
