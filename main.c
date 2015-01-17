#include <stdio.h>
#include <stdlib.h>
#include "rt_rtnetlink.h"

int main()
{
    struct ifinfomsg info;

    if (rt_get_info("eth0", &info) < 0) {
        perror("getinfo");
        exit(EXIT_FAILURE);
    }
    printf("getinfo complete\n");

    exit(EXIT_SUCCESS);
}
