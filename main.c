#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net/net_network.h"

int main(int argc, char **argv)
{
    int err = 0;

    if (argc < 3) {
        printf("missing arguments\n");
        exit(EXIT_FAILURE);
    }

    err = net_set_master(argv[1], argv[2]);

    if (err < 0) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
