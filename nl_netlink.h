#ifndef _NL_NETLINK_H
#define _NL_NETLINK_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

/**
 * struct nl_connection represent a netlink connection to the kernel.
 */
struct nl_connection {
    int fd;
    uint32_t seq;
    struct sockaddr_nl local;
};

/**
 * nl_connect opens a netlink socket.
 *
 * @return  NULL if the connection can't be established
 */
struct nl_connection *nl_connect(int protocol);

/**
 * nl_sens sends a message to the desired netlink address.
 *
 * @return  sequence number of the message just sent, -1 on error
 */
int nl_send(struct nl_connection *c, void *data, size_t len, uint16_t type,
        uint16_t flags, const struct sockaddr_nl *dst);

/**
 * nl_recv receives a message. The message return must be deallocated using the
 * nl_msg_free() function.
 *
 * @return  length of the message on successful completion, < 0 on error
 */
ssize_t nl_recv(struct nl_connection *c, void *buf, size_t len,
        struct sockaddr_nl *src_addr, socklen_t *addrlen);

/**
 * nl_close closes the netlink socket and frees the memory.
 *
 * @return  0 on success, -1 on failure
 */
int nl_close(struct nl_connection *conn);

#endif /* _NL_NETLINK_H */
