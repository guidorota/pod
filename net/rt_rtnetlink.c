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

struct ifinfomsg *rt_get_ifinfo(int index)
{
    struct nl_connection *c;
    struct ifinfomsg req;
    struct nlmsghdr *buf;
    size_t buflen = sysconf(_SC_PAGESIZE);

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
        return NULL;
    }

    if (!NLMSG_OK(buf, buflen)) {
        goto clean_buffer;
    }
    if (!rt_check_addr(&src_addr, addrlen)) {
        goto clean_buffer;
    }

    nl_close(c);

    return 0;

clean_buffer:
    free(buf);
    return NULL;
}


static ssize_t nl_sync(struct nl_connection *c, const void *snd_buf,
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
