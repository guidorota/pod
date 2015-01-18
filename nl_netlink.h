#ifndef _NL_NETLINK_H
#define _NL_NETLINK_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

struct nl_message {
    uint16_t type;
    uint16_t flags;
    void *data;
    size_t cap;
    size_t len;
};

/**
 * nl_msg_create creates a new netlink message to be used with this library.
 * 
 * @param   cap is the initial capacity, in bytes, of the message. Can be later
 *          expanded using the nl_msg_append function
 * @return  NULL if error occurs while allocating the required memory
 */
struct nl_message *nl_msg_create(size_t cap);

/**
 * nl_msg_append appends the data passed as parameter to the message, expanding
 * its size if needed.
 *
 * @return  0 on success, < 0 on failure
 */
int nl_msg_append(struct nl_message *msg, const void *data, const size_t len);

/**
 * nl_msg_free frees the memory occupied by the message
 */
void nl_msg_free(struct nl_message *msg);

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
int nl_send(struct nl_connection *c, const struct sockaddr_nl *dst,
        const struct nl_message *msg);

/**
 * nl_recv receives a message. The message return must be deallocated using the
 * nl_msg_free() function.
 *
 * @return  NULL if an error occurs while receiving
 */
struct nl_message *nl_recv(struct nl_connection *c,
        struct sockaddr_nl *src_addr, socklen_t *addrlen);

/**
 * nl_close closes the netlink socket and frees the memory.
 *
 * @return  0 on success, -1 on failure
 */
int nl_close(struct nl_connection *conn);

#endif /* _NL_NETLINK_H */
