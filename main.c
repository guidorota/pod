#include <stdio.h>
#include <stdlib.h>
#include "net_network.h"

int main()
{
    printf("eth0 up: %d\n", net_if_up("eth0"));
    exit(EXIT_SUCCESS);
}
