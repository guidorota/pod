#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include "net_network.h"
#include "rt_rtnetlink.h"

static bool net_check_ifname(const char *ifname);
static int net_ifindex(const char *ifname);

int net_if_up(char *ifname)
{
    int i;
    int up;
    struct rt_ifinfo *info;

    i = net_ifindex(ifname);
    if (i < 0) {
        return -1;
    }

    info = rt_get_ifinfo(i);
    if (info == NULL) {
        return -1;
    }

    up = info->info.ifi_flags & IFF_UP;
    rt_ifinfo_free(info);

    return up;
}

/**
 * net_ifindex returns the index of the interface whose name is passed as
 * parameter.
 *
 * @return  < 0 in case of error
 */
static int net_ifindex(const char *ifname)
{
    if (!net_check_ifname(ifname)) {
        return -1;
    } 

    return if_nametoindex(ifname);
}

/**
 * net_check_ifname checks if the string passed as parameter is a valid
 * interface name.
 */
static bool net_check_ifname(const char *ifname)
{
    int l;

    if (ifname == NULL) {
        return false;
    }

    l = strlen(ifname);
    if (l == 0 || l >= IF_NAMESIZE) {
        return false;
    }

    return true;
}
