#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net/net_network.h"

void print_usage(char *cmd)
{
    printf("Usage: %s [up|down|veth|bridge|address|delete|status] <if_name>\n", cmd);
}

int main_create_veth(char *basename)
{
    int err = -1;
    char *name1;
    char *name2;

    name1 = calloc(1, IF_NAMESIZE);
    if (name1 == NULL) {
        return -1;
    }
    strncpy(name1, basename, NET_NAMESIZE - 3);
    strcat(name1, "_0");

    name2 = calloc(1, IF_NAMESIZE);
    if (name2 == NULL) {
        free(name1);
        return -1;
    }
    strncpy(name2, basename, NET_NAMESIZE - 3);
    strcat(name2, "_1");
    err = net_create_veth(name1, name2);

    free(name1);
    free(name2);
    return err;
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
    } else if (strcmp(action, "veth") == 0) {
        err = main_create_veth(ifname);
    } else if (strcmp(action, "bridge") == 0) {
        err = net_create_bridge(ifname);
    } else if (strcmp(action, "delete") == 0) {
        err = net_delete(ifname);
    } else if (strcmp(action, "status") == 0) {
        printf("%s up: %d\n", ifname,  net_is_up(ifname));
    } else if (strcmp(action, "address") == 0) {
        err = net_add_ipv4(ifname, "135.100.100.100", 24);
    } else {
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);
    }

    if (err < 0) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
