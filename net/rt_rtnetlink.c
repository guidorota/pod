#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include "rt_rtnetlink.h"
#include "nl_netlink.h"

// kernel netlink address
const struct sockaddr_nl kernel = { AF_NETLINK, 0, 0, 0 };

// communication utility functions
static ssize_t rt_sync(const void *req_buf, size_t req_len, uint16_t type,
        uint16_t flags, struct nlmsghdr *reply_buf, size_t reply_len);
static bool rt_is_kernel(const struct sockaddr_storage *addr,
        socklen_t addrlen);

/*
int rt_create_veth(const char *name1, const char *name2)
{
    struct ifinfomsg req;
    int nl_flags;

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_change = 0xFFFFFFFF;

    nl_flags = NLM_F_CREATE | NLM_F_EXCL | NLM_F_REQUEST | NLM_F_ACK;

    return 0;
}
*/

int rt_delete_link(int index)
{
    struct ifinfomsg req;
    struct nlmsghdr *resp;
    ssize_t recvd;
    int err = 0;

    resp = calloc(RT_DGRAM_SIZE, 1);
    if (resp == NULL) {
        return -1;
    }

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_change = 0xFFFFFFFF;
    req.ifi_index = index;

    recvd = rt_sync(&req, sizeof req, RTM_DELLINK,
                NLM_F_REQUEST | NLM_F_ACK, resp, RT_DGRAM_SIZE);
    if (recvd < 0) {
        err = -1;
        goto clean_buf;
    }

    if (NL_ISERROR(resp)) {
        errno = -NL_ERROR_NO(resp);
        return -1;
    }

    if (!NL_ISACK(resp)) {
        err = -1;
        goto clean_buf;
    }

clean_buf:
    free(resp);
    return err;
}

int rt_set_link_flags(int index, uint32_t flags)
{
    struct ifinfomsg req;
    struct nlmsghdr *reply;
    ssize_t recvd;
    int err = 0;

    reply = calloc(RT_DGRAM_SIZE, 1);
    if (reply == NULL) {
        return -1;
    }

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_change = 0xFFFFFFFF;
    req.ifi_index = index;
    req.ifi_flags = flags;

    recvd = rt_sync(&req, sizeof req, RTM_NEWLINK,
                NLM_F_REQUEST | NLM_F_ACK, reply, RT_DGRAM_SIZE);
    if (recvd < 0) {
        err = -1;
        goto clean_buf;
    }

    if (NL_ISERROR(reply)) {
        errno = -NL_ERROR_NO(reply);
        return -1;
    }

    if (!NL_ISACK(reply)) {
        err = -1;
        goto clean_buf;
    }

clean_buf:
    free(reply);
    return err;
}

ssize_t rt_link_info(int index, void *buf, size_t len)
{
    struct ifinfomsg req;
    struct nlmsghdr *reply;
    ssize_t recvd;

    if (buf == NULL) {
        errno = EINVAL;
        return -1;
    }
    reply = buf;

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_change = 0xFFFFFFFF;
    req.ifi_index = index;

    recvd = rt_sync(&req, sizeof req, RTM_GETLINK, NLM_F_REQUEST, buf, len);
    if (recvd < 0) {
        return -1;
    }

    if (!NLMSG_OK(reply, recvd)) {
        errno = -EOVERFLOW;
        return -1;
    }

    if (NL_ISERROR(reply)) {
        errno = -NL_ERROR_NO(reply);
        return -1;
    }

    memmove(buf, NLMSG_DATA(reply), reply->nlmsg_len);

    return recvd;
}

/**
 * rt_sync sends a message to the kernel and synchronously receives a single
 * netlink reply message.
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

    recvd = nl_recv(c, reply_buf, reply_len,
                 (struct sockaddr *) &addr, &addrlen);
    if (recvd < 0) {
        goto close_conn;
    }

    if (!rt_is_kernel(&addr, addrlen)) {
        recvd = -1;
        goto close_conn;
    }
    if (reply_buf->nlmsg_seq != (uint32_t) seq) {
        recvd = -1;
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
