#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net/net_network.h"

void print_usage(char *cmd)
{
    printf("Usage: %s [up|down|delete|status] <if_name>\n", cmd);
}

int main(int argc, char **argv)
{
    char *ifname;
    char *action;
    int err = 0;

    if (argc < 3) {
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);
    }

    action = argv[1];
    ifname = argv[2];

    if (strcmp(action, "up") == 0) {
        err = net_up(ifname);
    } else if (strcmp(action, "down") == 0) {
        err = net_down(ifname);
    } else if (strcmp(action, "delete") == 0) {
        err = net_delete(ifname);
    } else if (strcmp(action, "status") == 0) {
        printf("%s up: %d\n", ifname,  net_is_up(ifname));
    } else {
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);
    }

    if (err < 0) {
        printf("error\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
