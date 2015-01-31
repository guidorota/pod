#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "net/rt_rtnetlink.h"

int main()
{
    rt_get_all_addr();
    return 0;
}
