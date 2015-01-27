#ifndef _NET_NETWORK_H
#define _NET_NETWORK_H

#include <net/if.h>

#include "rt_rtnetlink.h"

#define NET_NAMESIZE IF_NAMESIZE

struct net_info {
    struct ifinfomsg info;
    struct rtattr *atts[IFLA_MAX];
};

/**
 * net_info_free frees the memory occupied by a struct net_info
 */
void net_info_free(struct net_info *i);

/**
 * net_info retrieves information regarding a specific network interface.
 *
 * @return  NULL on failure
 */
struct net_info *net_info(char *ifname);

/**
 * net_addr_add_ipv4 adds an IPv4 address to the specified interface. This
 * function also sets a broadcast address with value (address | ~subnet_mask).
 *
 * @return  0 on success, -1 on failure
 */
int net_addr_add_ipv4(char *ifname, char *addr, unsigned char prefix);

/**
 * net_create_veth creates a new veth pair.
 *
 * @name name of the first endpoint
 * @peer_name name of the second endpoint
 *
 * @return 0 on successful creation, -1 on failure
 */
int net_create_veth(const char *name, const char *peer_name);

/**
 * net_create_bridge creates a new bridge.
 *
 * @return 0 on successful creation, -1 on failure
 */
int net_create_bridge(const char *name);

/**
 * net_delete deletes a network interface.
 *
 * @return  0 on success, -1 on failure
 */
int net_delete(char *ifname);

/**
 * net_is_up indicates if the interface whose name is passed as parameter is UP
 * or DOWN.
 *
 * @return  0 if the interface is down, 1 if it's up and -1 on error
 */
int net_is_up(char *ifname);

/**
 * net_up brings an interface up.
 *
 * @return  0 on success, -1 on failure
 */
int net_up(char *ifname);

/**
 * net_down brings an interface down
 *
 * @return  0 on success, -1 on failure
 */
int net_down(char *ifname);

#endif /* _NET_NETWORK_H */
