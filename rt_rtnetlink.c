#include <stdio.h>

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

static bool rt_check_addr(const struct sockaddr_storage *addr,
        socklen_t addrlen);

int rt_get_ifinfo(int index, struct ifinfomsg *info)
{
    struct nl_connection *c;
    struct sockaddr_storage src_addr;
    socklen_t addrlen;
    struct ifinfomsg req;
    struct nlmsghdr *buf;
    size_t datalen = sysconf(_SC_PAGESIZE);
    ssize_t len;

    memset(&req, 0, sizeof req);
    req.ifi_family = AF_UNSPEC;
    req.ifi_change = 0xFFFFFFFF;
    req.ifi_index = index;
    if (req.ifi_index == 0) {
        return -errno;
    }

    c = nl_connect(NETLINK_ROUTE);
    if (c == NULL) {
        return -1;
    }

    len = nl_send(c, &req, sizeof req, RTM_GETLINK, NLM_F_REQUEST, &kernel);
    if (len < 0) {
        return -1;
    }

    buf = calloc(datalen, 1);
    if (buf == NULL) {
        return -1;
    }
    len = nl_recv(c, buf, datalen, (struct sockaddr *) &src_addr, &addrlen);
    if (len < 0) {
        return -1;
    }

    if (!NLMSG_OK(buf, datalen)) {
        return -1;
    }
    if (!rt_check_addr(&src_addr, addrlen)) {
        return -1;
    }

    // TEST STUFF                                                               
    struct nlmsghdr *hdr = buf; 
    for (; NLMSG_OK(hdr, len); hdr = NLMSG_NEXT(hdr, len)) {
        info = NLMSG_DATA(buf);

        printf("sizeof (struct nlmsghdr): %lu\n", sizeof (struct nlmsghdr));
        printf("nlmsghdr len: %d\n", hdr->nlmsg_len);
        printf("nlmsghdr seq: %d\n", hdr->nlmsg_seq);
        if (hdr->nlmsg_type == NLMSG_DONE) {
            printf("done\n");
        }
        printf("if_index: %d\n", info->ifi_index);

        struct rtattr *rta =
            (struct rtattr *) (((char *) hdr) + 32);
        size_t attrlen = len - NLMSG_ALIGN(sizeof (struct nlmsghdr)) -
            NLMSG_ALIGN(sizeof (struct ifinfomsg)) - 4;
        printf("aligned nlmsghdr: %lu\n", NLMSG_ALIGN(sizeof (struct nlmsghdr))); 
        printf("aligned ifinfomsg: %lu\n", NLMSG_ALIGN(sizeof (struct ifinfomsg))); 
        printf("attrlen: %zu\n", attrlen);
        for (; RTA_OK(rta, attrlen); rta = RTA_NEXT(rta, attrlen)) {
            switch(rta->rta_type) {
            case IFLA_UNSPEC:
                printf("unspecified\n");
                break;
            case IFLA_ADDRESS:
                printf("iface address\n");
                break;
            case IFLA_BROADCAST:
                printf("broadcast address\n");
                break;
            case IFLA_IFNAME:
                printf("device name\n");
                char *name;
                name = RTA_DATA(rta);
                printf("\t%s\n", name);
                break;
            case IFLA_MTU:
                printf("mtu\n");
                break;
            case IFLA_LINK:
                printf("link type\n");
                break;
            case IFLA_QDISC:
                printf("queuing discipline\n");
                break;
            case IFLA_STATS:
                printf("iface statistics\n");
                break;
            }
        }
    } 

    nl_close(c);

    return 0;
}

static bool rt_check_addr(const struct sockaddr_storage *addr,
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
