#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/socket.h>
#include "rt_rtnetlink.h"
#include "nl_netlink.h"

// kernel netlink address
const struct sockaddr_nl kernel = { AF_NETLINK, 0, 0, 0 };

static int rt_check_ifname(const char *ifname);
static bool rt_is_kernel(const struct sockaddr_nl *addr);

int rt_get_info(char *ifname, struct ifinfomsg *info)
{
    struct nl_connection *c;
    struct sockaddr_nl src_addr;
    socklen_t addrlen;
    struct ifinfomsg req;
    struct nlmsghdr *data;
    size_t datalen = 2048;
    ssize_t len;

    assert(ifname != NULL && info != NULL);

    if (rt_check_ifname(ifname) != 0) {
        return -1;
    }

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_index = if_nametoindex(ifname);
    if (req.ifi_index == 0) {
        return -errno;
    }

    c = nl_connect(NETLINK_ROUTE);
    if (c == NULL) {
        return -1;
    }

    len = nl_send_data(c, &kernel, RTM_GETLINK,
            NLM_F_REQUEST, &req, sizeof req);
    if (len < 0) {
        return -1;
    }

    data = calloc(datalen, 1);
    if (data == NULL) {
        return -1;
    }
    nl_recv_from(c, data, datalen, &src_addr, &addrlen);

    if (!rt_is_kernel(&src_addr)) {
        return -1;
    }

    info = NLMSG_DATA(data);

    nl_close(c);

    return 0;
}

static int rt_check_ifname(const char *ifname)
{
    int l;

    assert(ifname != NULL);

    l = strlen(ifname);
    if (l == 0 || l >= IF_NAMESIZE) {
        return -1;
    }

    return 0;
}

static bool rt_is_kernel(const struct sockaddr_nl *addr)
{
    return addr->nl_family == AF_NETLINK && addr->nl_pid == 0;
}
