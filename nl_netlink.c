#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "nl_netlink.h"

#define NL_MSG_EMPTY(msg) (msg->data + msg->len)

static int nl_msg_expand(struct nl_message *msg, const size_t minlen);

static int nl_set_sock_buffer(struct nl_connection *c);
static int nl_read_local_sockaddr(struct nl_connection *c);
static struct nlmsghdr *nl_create_nlmsghdr(const void *data, size_t len,
        uint16_t type, uint16_t flags, uint32_t seq);

struct nl_message *nl_msg_create(size_t cap)
{
    struct nl_message *msg;

    msg = calloc(1, sizeof *msg);
    if (msg == NULL) {
        return NULL;
    }

    msg->data = calloc(1, cap);
    if (msg->data == NULL) {
        free(msg);
        return NULL;
    }

    return msg;
}

int nl_msg_append(struct nl_message *msg, const void *data, const size_t len)
{
    int err;

    if (msg == NULL || data == NULL || len == 0) {
        errno = EINVAL;
        return 0;
    } 

    if (msg->len + len > msg->cap) {
        err = nl_msg_expand(msg, len);
        if (err < 0) {
            return err;
        }
    }

    memcpy(NL_MSG_EMPTY(msg), data, len);

    return 0; 
}

static int nl_msg_expand(struct nl_message *msg, const size_t minlen)
{
    size_t newcap;

    if (2 * msg->cap + minlen > SIZE_MAX) {
        newcap = 2 * msg->cap + minlen;
    } else {
        newcap = SIZE_MAX;
    }

    if (newcap < msg->cap + minlen) {
        errno = ENOMEM;
        return -errno;
    }

    msg->data = realloc(msg->data, newcap);
    if (msg->data == NULL) {
        errno = ENOMEM;
        return -errno;
    }

    msg->cap = newcap;

    return 0;
}

void nl_msg_free(struct nl_message *msg)
{
    if (msg == NULL) {
        return;
    }

    if (msg->data != NULL) {
        free(msg->data);
    }

    free(msg);
}

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
        const struct nl_message* msg)
{
    uint16_t seq;
    struct nlmsghdr *nlmsg;

    if (c == NULL || dst == NULL) {
        errno = EINVAL;
        return -1;
    }

    seq = c->seq++;
    nlmsg = nl_create_nlmsghdr(msg->data, msg->len, msg->type,
            msg->flags, seq);
    if (nlmsg == NULL) {
        return -1;
    }

    if (sendto(c->fd, nlmsg, NLMSG_SPACE(msg->len), 0,
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

struct nl_message * nl_recv(struct nl_connection *c,
        struct sockaddr_nl *src_addr, socklen_t *addrlen)
{
    ssize_t recvd;
    struct nl_message *out;
    struct nlmsghdr *nlmsg;

    if (c == NULL || src_addr == NULL || addrlen == NULL) {
        errno = EINVAL;
        return NULL;
    }

    out = malloc(
    do {
        recvd = recvfrom(c->fd, data, len, 0,
                (struct sockaddr *) src_addr, addrlen);
    } while (recvd == EINTR);
    if (recvd < 0) {
        return NULL;
    }

    return out;
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
