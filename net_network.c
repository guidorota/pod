#include <net/if.h>
#include "net_network.h"
#include "rt_rtnetlink.h"

int net_if_up(char *name)
{
    struct ifinfomsg info;

    if (rt_get_info(name, &info) < 0) {
        return -1;
    }

    if (info.ifi_flags & 0x1) {
        return 1;
    }

    return 0;
}
