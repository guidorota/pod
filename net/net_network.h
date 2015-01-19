#ifndef _NET_NETWORK_H
#define _NET_NETWORK_H

int net_up(char *ifname);

int net_is_up(char *ifname);

int net_down(char *ifname);

#endif /* _NET_NETWORK_H */
