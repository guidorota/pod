#include <stdio.h>

#include <sys/socket.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>

#include "rt_rtnetlink.h"
#include "nl_netlink.h"
#include "../utils/dy_dynbuf.h"

/**
 * RT_ENC_FREE returns the first free byte in the encoder.
 */
#define RT_ENC_FREE(e) (void *) ((char *) e->buf + e->len)

struct rt_encoder *rt_enc_create()
{
    return rt_enc_create_cap(RT_DGRAM_SIZE);
}

struct rt_encoder *rt_enc_create_cap(size_t cap)
{
    struct rt_encoder *e;

    e = calloc(1, sizeof *e);
    if (e == NULL) {
        return NULL;
    }

    e->buf = calloc(cap, 1);
    if (e->buf == NULL) {
        free(e);
        return NULL;
    }
    e->cap = cap;

    return e;
}

int rt_enc_data(struct rt_encoder *e, const void *buf, size_t len)
{
    size_t alen;

    if (e == NULL || buf == NULL) {
        errno = EINVAL;
        return -1;
    }

    alen = NLMSG_ALIGN(len);
    if (e->len + alen > e->cap) {
        errno = EOVERFLOW;
        return -1;
    }

    memcpy(RT_ENC_FREE(e), buf, len);
    e->len += alen;

    return 0;
}

int rt_enc_attribute(struct rt_encoder *e, unsigned int type,
        const void *buf, size_t len)
{
    struct rtattr *rta;

    if (e->len + RTA_SPACE(len) > e->cap) {
        errno = EINVAL;
        return -1;
    }

    rta = RT_ENC_FREE(e);
    rta->rta_type = type;
    rta->rta_len = RTA_LENGTH(len);

    memcpy(RTA_DATA(rta), buf, len);
    e->len += RTA_SPACE(len);

    return 0;
}

void rt_enc_free(struct rt_encoder *e)
{
    free(e->buf);
    free(e);
}

// kernel netlink address
const struct sockaddr_nl kernel = { AF_NETLINK, 0, 0, 0 };

// helper functions
static struct dy_dynbuf *rt_multi_request(const void *req, size_t req_len,
        uint16_t type, uint16_t flags);
static int rt_simple_request(const void *req, size_t req_len, uint16_t type,
        uint16_t flags);
static ssize_t rt_sync(const void *req_buf, size_t req_len, uint16_t type,
        uint16_t flags, struct nlmsghdr *reply_buf, size_t reply_len);
static bool rt_is_kernel(const struct sockaddr_storage *addr,
        socklen_t addrlen);

struct dy_dynbuf *rt_get_all_addr(int index, int family)
{
    struct ifaddrmsg ifa;
    struct dy_dynbuf *buf;
    struct dy_dynbuf *resp;
    struct nlmsghdr *nlmsg;
    size_t nlmsg_len;
    size_t rtalen;

    struct ifaddrmsg *ifa_resp;
    struct rtattr *rta;

    printf("asdf: %d\n", index);

    memset(&ifa, 0, sizeof ifa);

    buf = rt_multi_request(&ifa, sizeof ifa, RTM_GETADDR, NLM_F_ROOT | NLM_F_REQUEST);
    if (buf == NULL) {
        return NULL;
    }

    resp = dy_create();
    if (resp == NULL) {
        goto err_free_buf;
    }

    nlmsg = buf->buf;
    nlmsg_len = buf->len;
    if (!NLMSG_OK(nlmsg, nlmsg_len)) {
        goto err_free_resp;
    }
    for (; NLMSG_OK(nlmsg, nlmsg_len); nlmsg = NLMSG_NEXT(nlmsg, nlmsg_len)) {
        ifa_resp = NLMSG_DATA(nlmsg); 
        printf("index: %d\n", ifa_resp->ifa_index);
        printf("family: %d\n", ifa_resp->ifa_family);
        rtalen = nlmsg->nlmsg_len - NLMSG_ALIGN(sizeof *nlmsg) -
            NLMSG_ALIGN(sizeof *ifa_resp);
        rta = (struct rtattr *) ((char *) ifa_resp + NLMSG_ALIGN(sizeof *ifa_resp));
        for (; RTA_OK(rta, rtalen); rta = RTA_NEXT(rta, rtalen)) {
            printf("type: %d\n", rta->rta_type);
        }
    }
    printf("---\n");

    return resp;

err_free_resp:
    dy_free(resp);
err_free_buf:
    dy_free(buf);
    return NULL;
}

int rt_addr_add(const struct ifaddrmsg *ifa, size_t ifa_len)
{
    return rt_simple_request(ifa, ifa_len, RTM_NEWADDR,
            NLM_F_CREATE | NLM_F_EXCL | NLM_F_REQUEST | NLM_F_ACK);
}

int rt_link_set_attribute(int index, unsigned int attr, const void *buf,
        size_t len)
{
    ssize_t recvd;
    unsigned char *info_buf[RT_DGRAM_SIZE];
    struct rt_encoder *enc;
    int err = -1;

    recvd = rt_link_info(index, info_buf, RT_DGRAM_SIZE);
    if (recvd < 0) {
        return -1;
    }

    enc = rt_enc_create();
    if (enc == NULL) {
        return -1;
    }
    if (rt_enc_data(enc, info_buf, sizeof (struct ifinfomsg)) < 0) {
        goto free_enc;
    }
    if (rt_enc_attribute(enc, attr, buf, len) < 0) {
        goto free_enc;
    }

    err = rt_simple_request(enc->buf, enc->len, RTM_NEWLINK,
            NLM_F_ACK | NLM_F_REQUEST);

free_enc:
    rt_enc_free(enc);
    return err;
}

int rt_link_create(const struct ifinfomsg *info, size_t info_len)
{
    return rt_simple_request(info, info_len, RTM_NEWLINK,
            NLM_F_CREATE | NLM_F_EXCL | NLM_F_REQUEST | NLM_F_ACK);
}

int rt_link_delete(int index)
{
    struct ifinfomsg req;

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_change = 0xFFFFFFFF;
    req.ifi_index = index;

    return rt_simple_request(&req, sizeof req, RTM_DELLINK,
            NLM_F_REQUEST | NLM_F_ACK);
}

int rt_link_set_flags(int index, uint32_t flags)
{
    struct ifinfomsg req;

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_change = 0xFFFFFFFF;
    req.ifi_index = index;
    req.ifi_flags = flags;

    return rt_simple_request(&req, sizeof req, RTM_NEWLINK,
            NLM_F_REQUEST | NLM_F_ACK);
}

ssize_t rt_link_info(int index, void *buf, size_t len)
{
    struct ifinfomsg req;
    struct nlmsghdr *resp;
    ssize_t recvd;

    if (buf == NULL) {
        errno = EINVAL;
        return -1;
    }
    resp = buf;

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_change = 0xFFFFFFFF;
    req.ifi_index = index;

    recvd = rt_sync(&req, sizeof req, RTM_GETLINK, NLM_F_REQUEST, buf, len);
    if (recvd < 0) {
        return -1;
    }

    if (!NLMSG_OK(resp, recvd)) {
        errno = -EOVERFLOW;
        return -1;
    }

    if (NL_ISERROR(resp)) {
        errno = -NL_ERROR_NO(resp);
        return -1;
    }

    // delete netlink header, move struct ifinfomsg at the beginning of the
    // buffer
    memmove(buf, NLMSG_DATA(resp), resp->nlmsg_len);

    return recvd;
}

static struct dy_dynbuf *rt_multi_request(const void *req, size_t req_len,
        uint16_t type, uint16_t flags)
{
    struct nl_connection *c;
    int seq;
    unsigned char recv_buf[RT_DGRAM_SIZE];
    struct nlmsghdr *recv_msg = (struct nlmsghdr *) recv_buf;
    struct dy_dynbuf *buf;
    ssize_t recv_len;
    struct sockaddr_storage addr;
    socklen_t addr_len;

    buf = dy_create();
    if (buf == NULL) {
        return NULL;
    }

    c = nl_connect(NETLINK_ROUTE);
    if (c == NULL) {
        goto err_free_buf;
    }

    seq = nl_send(c, req, req_len, type, flags | NLM_F_REQUEST, &kernel);
    if (seq == -1) {
        goto err_close_conn; 
    }

    // The following loop copies an entire netlink response inside a dynamic
    // buffer, even if it spans multiple netlink datagrams
    do {
        memset(&addr, 0, sizeof addr);
        addr_len = sizeof addr;
        recv_len = nl_recv(c, recv_msg, RT_DGRAM_SIZE,
                (struct sockaddr *) &addr, &addr_len);

        if (recv_len < 0) {
            goto err_close_conn;
        }

        if (!rt_is_kernel(&addr, addr_len) ||
                recv_msg->nlmsg_seq != (uint32_t) seq) {
            goto err_close_conn;
        }

        if (!NLMSG_OK(recv_msg, recv_len)) {
            goto err_close_conn;
        }
        if (NL_ISERROR(recv_msg)) {
            errno = -NL_ERROR_NO(recv_msg);
            goto err_close_conn;
        }

        if (dy_add(buf, recv_msg, recv_len) < 0) {
            goto err_close_conn;
        }

    } while(recv_msg->nlmsg_flags | NLM_F_MULTI &&
                recv_msg->nlmsg_type != NLMSG_DONE);

    nl_close(c);
    return buf;

err_close_conn:
    nl_close(c);
err_free_buf:
    dy_free(buf);
    return NULL;
}

/**
 * rt_simple_request sends an rtnetlink request message and parses the
 * acknowledgement reponse.
 */
static int rt_simple_request(const void *req, size_t req_len, uint16_t type,
        uint16_t flags)
{
    unsigned char resp[RT_DGRAM_SIZE];
    ssize_t recvd;

    recvd = rt_sync(req, req_len, type, flags | NLM_F_ACK,
                (struct nlmsghdr *) resp, RT_DGRAM_SIZE);
    if (recvd < 0) {
        return -1;
    }

    if (NL_ISERROR((struct nlmsghdr *) resp)) {
        errno = -NL_ERROR_NO(resp);
        return -1;
    }

    if (!NL_ISACK((struct nlmsghdr *) resp)) {
        return -1;
    }

    return 0;
}

/**
 * rt_sync sends a message to the kernel and synchronously receives a single
 * netlink response message.
 *
 * @return  bytes received, -1 on error
 */
static ssize_t rt_sync(const void *req_buf, size_t req_len, uint16_t type,
        uint16_t flags, struct nlmsghdr *reply_buf, size_t reply_len) {
    struct nl_connection *c;
    struct sockaddr_storage addr;
    socklen_t addrlen;
    int seq;
    ssize_t recvd;

    c = nl_connect(NETLINK_ROUTE);
    if (c == NULL) {
        return -1;
    }

    seq = nl_send(c, req_buf, req_len, type, flags, &kernel);
    if (seq < 0) {
        recvd = -1;
        goto close_conn;
    }

    memset(&addr, 0, sizeof addr);
    addrlen = sizeof addr;
    recvd = nl_recv(c, reply_buf, reply_len,
                 (struct sockaddr *) &addr, &addrlen);
    if (recvd < 0) {
        goto close_conn;
    }

    if (!rt_is_kernel(&addr, addrlen) ||
            reply_buf->nlmsg_seq != (uint32_t) seq) {
        recvd = -1;
        goto close_conn;
    }

close_conn:
    nl_close(c);
    return recvd;
}

/**
 * rt_is_kernel checks if the address structure passed as parameter is a valid
 * netlink kernel address.
 */
static bool rt_is_kernel(const struct sockaddr_storage *addr,
        socklen_t addrlen)
{
    if (addrlen != sizeof (struct sockaddr_nl)) {
        return -1;
    }

    if (((struct sockaddr_nl *)addr)->nl_pid != 0) {
        return false;
    }
    return true;
}
