#include <assert.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "nl_netlink.h"

static int nl_set_sock_buffer(struct nl_connection *c);
static int nl_read_local_sockaddr(struct nl_connection *c);
static struct nlmsghdr *nl_create_nlmsghdr(const void *data, size_t len,
        uint16_t type, uint16_t flags, uint32_t seq);
static void nl_free_nlmsghdr(struct nlmsghdr *nlmsg);

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

int nl_send(struct nl_connection *c, const struct sockaddr_nl *dst,
        uint16_t type, uint16_t flags)
{
    return nl_send_data(c, dst, type, flags, NULL, 0);
}

int nl_send_data(struct nl_connection *c, const struct sockaddr_nl *dst,
        uint16_t type, uint16_t flags, const void *data, size_t len)
{
    ssize_t sent;
    struct nlmsghdr *nlmsg;

    assert(c != NULL && dst != NULL);

    nlmsg = nl_create_nlmsghdr(data, len, type, flags, c->seq++);
    if (nlmsg == NULL) {
        return -1;
    }

    sent = sendto(c->fd, nlmsg, NLMSG_SPACE(len), 0,
            (struct sockaddr *) dst, sizeof *dst);
    if (sent < 0) {
        return -1;
    }

    nl_free_nlmsghdr(nlmsg);
    
    return sent;
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

static void nl_free_nlmsghdr(struct nlmsghdr *nlmsg)
{
    free(nlmsg);
}

ssize_t nl_recv_from(struct nl_connection *c, struct nlmsghdr *data,
        size_t len, struct sockaddr_nl *src_addr, socklen_t *addrlen)
{
    ssize_t recvd;

    assert(c != NULL && data != NULL && src_addr != NULL && addrlen != NULL);

    do {
        recvd = recvfrom(c->fd, data, len, 0,
                (struct sockaddr *) src_addr, addrlen);
    } while (recvd == EINTR);

    if (recvd < 0) {
        return -errno;
    }

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
