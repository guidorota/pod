#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include "net_network.h"
#include "rt_rtnetlink.h"

static int net_set_flags(char *ifname, uint32_t set, uint32_t unset);
static bool net_check_ifname(const char *ifname);
static int net_ifindex(const char *ifname);

int net_delete(char *ifname)
{
    int i; 
    i = net_ifindex(ifname);
    if (i < 0) {
        return -1;
    }

    return rt_delete_link(i);
}

int net_up(char *ifname)
{
    return net_set_flags(ifname, IFF_UP, 0);
}

int net_down(char *ifname)
{
    return net_set_flags(ifname, 0, IFF_UP);
}

/**
 * net_set_flags changes the flags of the interface passed as parameter.
 *
 * @return  0 on success, -1 on failure
 */
static int net_set_flags(char *ifname, uint32_t set, uint32_t unset)
{
    int i;
    uint32_t flags;
    struct ifinfomsg *info;
    ssize_t info_len;

    i = net_ifindex(ifname);
    if (i < 0) {
        return -1;
    }

    info = calloc(RT_DGRAM_SIZE, 1);
    if (info == NULL) {
        return -1;
    }

    info_len = rt_link_info(i, info, RT_DGRAM_SIZE);
    if (info_len < 0) {
        goto free_buffer;
    }

    flags = info->ifi_flags;
    flags |= set;
    flags &= ~unset;

    if (rt_set_link_flags(i, flags) < 0) {
        goto free_buffer;
    }

    return 0;

free_buffer:
    free(info);
    return -1;
}

int net_is_up(char *ifname)
{
    int i;
    int up;
    struct ifinfomsg *info;
    ssize_t info_len;

    i = net_ifindex(ifname);
    if (i < 0) {
        return -1;
    }

    info = calloc(RT_DGRAM_SIZE, 1);
    if (info == NULL) {
        return -1;
    }

    info_len = rt_link_info(i, info, RT_DGRAM_SIZE);
    if (info_len < 0) {
        free(info);
        return -1;
    }

    return info->ifi_flags & IFF_UP;
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
