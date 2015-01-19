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

static ssize_t rt_sync(struct nl_connection *c, const void *snd_buf,
        size_t snd_len, uint16_t type, uint16_t flags, void *rcv_buf,
        size_t rcv_len);
static bool rt_is_kernel(const struct sockaddr_storage *addr,
        socklen_t addrlen);
static struct rt_info *rt_parse_ifinfomsg(struct nlmsghdr *msg, size_t len);
static struct rtattr *rt_copy_rta(struct rtattr *rta);

struct rt_ifinfo {
    struct ifinfomsg info;
    struct rtattr *atts[IFLA_MAX];
};

void rt_free_ifinfomsg(struct ifinfomsg *info)
{
    if (info == NULL) {
        return;
    }

    free(info);
}

struct ifinfomsg *rt_get_ifinfo(int index)
{
    struct nl_connection *c;
    struct ifinfomsg req;
    struct ifinfomsg *ret = NULL;
    struct nlmsghdr *buf;
    size_t buflen = sysconf(_SC_PAGESIZE);
    ssize_t recvd;

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_change = 0xFFFFFFFF;
    req.ifi_index = index;

    c = nl_connect(NETLINK_ROUTE);
    if (c == NULL) {
        return NULL;
    }

    buf = calloc(buflen, 1);
    if (buf == NULL) {
        goto clean_connection;
    }

    recvd = rt_sync(c, &req, sizeof req, RTM_GETLINK, 
                NLM_F_REQUEST, buf, buflen);
    if (recvd < 0) {
        goto clean_buffer;
    }


clean_buffer:
    free(buf);
clean_connection:
    nl_close(c);
    return ret;
}

static struct rt_info *rt_parse_ifinfomsg(struct nlmsghdr *msg, size_t len)
{
    struct ifinfomsg *ifinfo;
    struct rtattr *rta;
    struct rt_ifinfo *info;

    if (msg == NULL ||
            len < NLMSG_ALIGN(sizeof (struct nlmsghdr)) +
                NLMSG_ALIGN(sizeof (struct ifinfomsg))) {
        errno = EINVAL;
        return NULL;
    }

    if (NL_ISERROR(msg)) {
        errno = NL_ERROR_NO(msg);
        return NULL;
    }

    info = calloc(1, sizeof *info);
    if (info == NULL) {
        return NULL;
    }

    ifinfo = NLMSG_DATA(msg);
    memcpy(&info->info, ifinfo, sizeof info->info); 
    
    rta = (struct rtattr *) ((char *) ifinfo + NLMSG_ALIGN(sizeof *ifinfo));
    len -= NLMSG_ALIGN(sizeof (struct nlmsghdr)) + NLMSG_ALIGN(sizeof (struct ifinfomsg));
    for (; RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
        info->atts[rta->rta_type] = rt_copy_rta(rta);
    }
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
static ssize_t rt_sync(struct nl_connection *c, const void *snd_buf,
        size_t snd_len, uint16_t type, uint16_t flags, void *rcv_buf,
        size_t rcv_len) {
    struct sockaddr_storage addr;
    socklen_t addrlen;
    ssize_t len;

    len = nl_send(c, &snd_buf, snd_len, type, flags, &kernel);
    if (len < 0) {
        return -1;
    }

    len = nl_recv(c, rcv_buf, rcv_len, (struct sockaddr *) &addr, &addrlen);
    if (len < 0) {
        return -1; 
    }

    if (!rt_is_kernel(&addr, addrlen)) {
        return -1;
    }

    return len;
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
