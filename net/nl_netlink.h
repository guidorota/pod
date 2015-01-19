#ifndef _NL_NETLINK_H
#define _NL_NETLINK_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

/**
 * NL_ERROR_NO returns the error number stored in a NLMSG_ERROR netlink
 * message.
 */
#define NL_ERROR_NO(hdr) (((struct nlmsgerr *)NLMSG_DATA(hdr))->error)

/**
 * NL_ISERROR returns true if the netlink message is of error type.
 */
#define NL_ISERROR(hdr)                    \
        (hdr->nlmsg_type == NLMSG_ERROR && \
        NL_ERROR_NO(hdr) != 0)

/**
 * NL_ISACK returns true if the netlink message is an ACK.
 */
#define NL_ISACK(HDR)                      \
        (hdr->nlmsg_type == NLMSG_ERROR && \
        NL_ERROR_NO(hdr) == 0)

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
 * nl_send_raw sends the netlink message passed as parameter to the desired
 * destination.
 *
 * @return  sequence number of the message sent, -1 on error
 */
int nl_send_raw(struct nl_connection *c, struct nlmsghdr *hdr,
        const struct sockaddr_nl *dst);

/**
 * nl_send encapsulates the data passed as parameter in a netlink message and
 * sends it to the desired netlink address.
 *
 * @return  sequence number of the message sent, -1 on error
 */
int nl_send(struct nl_connection *c, void *buf, size_t len, uint16_t type,
        uint16_t flags, const struct sockaddr_nl *dst);

/**
 * nl_recv receives a message into the buf parameter.
 * It does not perform any sanity checks on the message received. The caller is
 * responsible for allocating a buffer large enough to contain the data to be
 * received; failure to do so may result in a truncated message.
 *
 * @return  length of the message on successful completion, < 0 on error
 */
ssize_t nl_recv(struct nl_connection *c, struct nlmsghdr *buf, size_t len,
        struct sockaddr *src_addr, socklen_t *addrlen);

/**
 * nl_close closes the netlink socket and frees the memory.
 *
 * @return  0 on success, -1 on failure
 */
int nl_close(struct nl_connection *conn);

#endif /* _NL_NETLINK_H */
