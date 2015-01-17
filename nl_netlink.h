#ifndef _NL_NETLINK_H
#define _NL_NETLINK_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

/*
 * struct nl_connection represent a netlink connection to the kernel.
 */
struct nl_connection {
    int fd;
    uint32_t seq;
    struct sockaddr_nl local;
};

struct nl_connection *nl_connect(int protocol);

int nl_send(struct nl_connection *c, const struct sockaddr_nl *dst,
        uint16_t type, uint16_t flags);

int nl_send_data(struct nl_connection *c, const struct sockaddr_nl *dst,
        uint16_t type, uint16_t flags, const void *data, size_t len);

ssize_t nl_recv_from(struct nl_connection *c, struct nlmsghdr *data,
        size_t len, struct sockaddr_nl *src_addr, socklen_t *addrlen);

int nl_close(struct nl_connection *conn);

#endif /* _NL_NETLINK_H */
