#ifndef _NET_NETWORK_H
#define _NET_NETWORK_H

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
