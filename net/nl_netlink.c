#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "nl_netlink.h"

#define NL_MSG_EMPTY(msg) (msg->data + msg->len)

static int nl_set_sock_buffer(struct nl_connection *c);
static int nl_read_local_sockaddr(struct nl_connection *c);
static struct nlmsghdr *nl_create_nlmsghdr(const void *data, size_t len,
        uint16_t type, uint16_t flags, uint32_t seq);

struct nl_connection *nl_connect(int protocol)
{
    struct nl_connection *c;

    c = calloc(1, sizeof (struct nl_connection));
    if (c == NULL) {
        return NULL;
    }

    c->fd = socket(AF_NETLINK, SOCK_RAW, protocol);
    if (c->fd < 0) {
        goto cleanup;
    }

    if (nl_set_sock_buffer(c) < 0) {
        goto cleanup;
    }

    c->local.nl_family = AF_NETLINK;
    if (bind(c->fd, (struct sockaddr *) &c->local, sizeof c->local) < 0) {
        goto cleanup;
    }

    if (nl_read_local_sockaddr(c) < 0) {
        goto cleanup;
    }

    return c;

cleanup:
    free(c);
    return NULL;
}

static int nl_set_sock_buffer(struct nl_connection *c)
{
    int buflen = 32768;

    if (setsockopt(c->fd, SOL_SOCKET, SO_SNDBUF, &buflen, sizeof buflen) < 0) {
        return -errno;
    }
    if (setsockopt(c->fd, SOL_SOCKET, SO_SNDBUF, &buflen, sizeof buflen) < 0) {
        return -errno;
    }
    
    return 0;
}

/*
 * nl_read_local_sockaddr reads the local socket address.  This function is
 * intended to be used to retrieve the kernel-assigned pid after binding the
 * socket.
 */
static int nl_read_local_sockaddr(struct nl_connection *c) { socklen_t socklen;
    
    socklen = sizeof c->local;
    if (getsockname(c->fd, (struct sockaddr *) &c->local, &socklen) < 0) {
        return -1;
    }
    if (socklen != sizeof c->local || c->local.nl_family != AF_NETLINK) {
        return -1;
    }

    return 0;
}

int nl_send_raw(struct nl_connection *c, const struct nlmsghdr *hdr,
        const struct sockaddr_nl *dst)
{
    uint16_t seq;

    if (c == NULL || hdr == NULL || dst == NULL) {
        errno = EINVAL;
        return -1;
    }

    seq = c->seq++;
    if (sendto(c->fd, hdr, NLMSG_ALIGN(hdr->nlmsg_len), 0,
            (struct sockaddr *) dst, sizeof *dst) < 0) {
        return -1;
    }

    return seq; 
}

int nl_send(struct nl_connection *c, const void *buf, size_t len,
        uint16_t type, uint16_t flags, const struct sockaddr_nl *dst)
{
    uint16_t seq;
    struct nlmsghdr *nlmsg;

    if (c == NULL || dst == NULL) {
        errno = EINVAL;
        return -1;
    }

    seq = c->seq++;
    nlmsg = nl_create_nlmsghdr(buf, len, type, flags, seq);
    if (nlmsg == NULL) {
        return -1;
    }

    if (sendto(c->fd, nlmsg, NLMSG_SPACE(len), 0,
            (struct sockaddr *) dst, sizeof *dst) < 0) {
        seq = -1;
    }

    free(nlmsg);
    
    return seq;
}

static struct nlmsghdr *nl_create_nlmsghdr(const void *data, size_t len,
        uint16_t type, uint16_t flags, uint32_t seq)
{
    struct nlmsghdr *nlmsg;
    size_t msg_len;

    msg_len = NLMSG_SPACE(len);
    nlmsg = calloc(1, msg_len);
    if (nlmsg == NULL) {
        return NULL;
    }

    nlmsg->nlmsg_type = type;
    nlmsg->nlmsg_flags = flags;
    nlmsg->nlmsg_len = NLMSG_LENGTH(len);
    nlmsg->nlmsg_seq = seq;

    if (len > 0) {
        memcpy(NLMSG_DATA(nlmsg), data, len);
    }

    return nlmsg;
}

ssize_t nl_recv(struct nl_connection *c, struct nlmsghdr *buf, size_t len,
        struct sockaddr *src_addr, socklen_t *addrlen)
{
    ssize_t recvd;

    if (c == NULL || src_addr == NULL || addrlen == NULL) {
        errno = EINVAL;
        return -1; 
    }

    do {
        recvd = recvfrom(c->fd, buf, len, 0, src_addr, addrlen);
    } while (recvd == EINTR);

    return recvd;
}

int nl_close(struct nl_connection *c)
{
    int err = 0;

    if (c == NULL) {
        return -1;
    }

    err = close(c->fd);
    free(c);
    return err;
}
