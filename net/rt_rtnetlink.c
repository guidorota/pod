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

/**
 * Offset of the first rtattr byte inside a netlink packet containing a struct
 * ifinfomsg.
 */
#define RT_RTA_OFFSET (NLMSG_ALIGN(sizeof (struct nlmsghdr)) + \
        NLMSG_ALIGN(sizeof (struct ifinfomsg)))

/**
 * Address of the first rtattr in a netlink packet containing a struct
 * ifinfomsg.
 */
#define RT_RTA_FIRST(hdr) ((struct rtattr *) ((char *) hdr + RT_RTA_OFFSET))


// kernel netlink address
const struct sockaddr_nl kernel = { AF_NETLINK, 0, 0, 0 };

static ssize_t rt_sync(const void *req_buf, size_t req_len, uint16_t type,
        uint16_t flags, struct nlmsghdr *reply_buf, size_t reply_len);
static bool rt_is_kernel(const struct sockaddr_storage *addr,
        socklen_t addrlen);
static struct rt_ifinfo *rt_parse_ifinfomsg(struct nlmsghdr *msg, size_t len);
static struct rtattr *rt_copy_rta(struct rtattr *rta);

void rt_ifinfo_free(struct rt_ifinfo *ifinfo)
{
    int i;

    if (ifinfo == NULL) {
        return;
    }

    for (i = 0; i < IFLA_MAX; i++) {
        if (ifinfo->atts[i] == NULL) {
            continue;
        }
        free(ifinfo->atts[i]);
    }

    free(ifinfo);
}

int rt_delete(int index)
{
    struct ifinfomsg req;
    struct nlmsghdr *resp;
    ssize_t buflen = sysconf(_SC_PAGESIZE);
    ssize_t recvd;
    int err = 0;

    resp = calloc(buflen, 1);
    if (resp == NULL) {
        return -1;
    }

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_change = 0xFFFFFFFF;
    req.ifi_index = index;

    recvd = rt_sync(&req, sizeof req, RTM_DELLINK,
                NLM_F_REQUEST | NLM_F_ACK, resp, buflen);
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

int rt_set_flags(int index, uint32_t flags)
{
    struct ifinfomsg req;
    struct nlmsghdr *hdr;
    size_t buflen = sysconf(_SC_PAGESIZE);
    ssize_t recvd;
    int err = 0;

    hdr = calloc(buflen, 1);
    if (hdr == NULL) {
        return -1;
    }

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_change = 0xFFFFFFFF;
    req.ifi_index = index;
    req.ifi_flags = flags;

    recvd = rt_sync(&req, sizeof req, RTM_NEWLINK,
                NLM_F_REQUEST | NLM_F_ACK, hdr, buflen);
    if (recvd < 0) {
        err = -1;
        goto clean_buf;
    }

    if (NL_ISERROR(hdr)) {
        errno = -NL_ERROR_NO(hdr);
        return -1;
    }

    if (!NL_ISACK(hdr)) {
        err = -1;
        goto clean_buf;
    }

clean_buf:
    free(hdr);
    return err;
}

struct rt_ifinfo *rt_get_ifinfo(int index)
{
    struct ifinfomsg req;
    struct rt_ifinfo *info = NULL;
    struct nlmsghdr *buf;
    size_t buflen = sysconf(_SC_PAGESIZE);
    ssize_t recvd;

    buf = calloc(buflen, 1);
    if (buf == NULL) {
        return NULL;
    }

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_change = 0xFFFFFFFF;
    req.ifi_index = index;

    recvd = rt_sync(&req, sizeof req, RTM_GETLINK, NLM_F_REQUEST, buf, buflen);
    if (recvd < 0) {
        goto clean_buffer;
    }

    info = rt_parse_ifinfomsg(buf, recvd);
    if (info == NULL) {
        goto clean_buffer;
    }

clean_buffer:
    free(buf);
    return info;
}

/**
 * rt_parse_ifinfomsg parses interface information from a ifinfomsg rtnetlink
 * structure.
 *
 * @return  NULL in case of error
 */
static struct rt_ifinfo *rt_parse_ifinfomsg(struct nlmsghdr *hdr, size_t len)
{
    struct rtattr *rta;
    struct rtattr *rta_copy;
    struct rt_ifinfo *info;

    if (hdr == NULL || len < RT_RTA_OFFSET) {
        errno = EINVAL;
        return NULL;
    }

    if (NL_ISERROR(hdr)) {
        errno = -NL_ERROR_NO(hdr);
        return NULL;
    }

    info = calloc(1, sizeof *info);
    if (info == NULL) {
        return NULL;
    }

    memcpy(&info->info, NLMSG_DATA(hdr), sizeof info->info); 
    
    rta = RT_RTA_FIRST(hdr);
    len -= RT_RTA_OFFSET;
    for (; RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
        rta_copy = rt_copy_rta(rta);
        if (rta_copy == NULL) {
            goto free_ifinfo;
        }
        info->atts[rta->rta_type] = rta_copy;
    }

    return info;

free_ifinfo:
    rt_ifinfo_free(info);
    return NULL;
}

/**
 * rt_copy_rta creates a copy of a rtnetlink interface attribute.
 *
 * @return  NULL in case of error
 */
static struct rtattr *rt_copy_rta(struct rtattr *rta)
{
    struct rtattr *copy;

    if (rta == NULL) {
        return NULL;
    }
    
    copy = malloc(rta->rta_len);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, rta, rta->rta_len);
    return copy;
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

    recvd = nl_recv(c, reply_buf, reply_len, (struct sockaddr *) &addr, &addrlen);
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
