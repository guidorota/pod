#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/veth.h>
#include "net_network.h"
#include "rt_rtnetlink.h"

#define NET_LINK_VETH "veth"
#define NET_LINK_BRIDGE "bridge"

static struct net_info *net_info_decode(void *buf, size_t len);
static struct rtattr *net_rta_copy(struct rtattr *rta);

static struct rt_encoder *net_ipv4_req(int index, const void *addr,
        const void *bcast, unsigned char prefix);
static int net_set_flags(char *ifname, uint32_t set, uint32_t unset);
static bool net_check_ifname(const char *ifname);

/**
 * NET_IPV4_LEN corresponds to the size of an IPv4 address.
 */
#define NET_IPV4_LEN (sizeof (struct in_addr))

/**
 * NET_IPV4_BCAST computes the broadcast address. 
 */
#define NET_IPV4_BCAST(addr, pre) (addr.s_addr | (0xFFFFFFFF << prefix))

struct net_info *net_info(char *ifname)
{
    int i;
    unsigned char buf[RT_DGRAM_SIZE];
    ssize_t read;

    i = net_ifindex(ifname);
    if (i < 0) {
        return NULL;
    }

    read = rt_link_info(i, &buf, RT_DGRAM_SIZE);
    if (read < 0) {
        return NULL;
    }

    return net_info_decode(buf, read);
}

static struct net_info *net_info_decode(void *buf, size_t len)
{
    struct net_info *info;
    struct rtattr *rta;
    struct rtattr *copy;
    size_t rta_len;

    info = calloc(1, sizeof *info);
    if (info == NULL) {
        return NULL;
    }

    if (len < sizeof info->info) {
        return NULL;
    }

    memcpy(&info->info, buf, sizeof info->info);

    rta = RT_RTA(buf);
    rta_len = RT_RTA_LEN(len);
    for (; RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
        copy = net_rta_copy(rta);
        if (copy == NULL) {
            goto err;
        } 
        info->atts[rta->rta_type] = copy;
    }
    
    return info;

err:
    net_info_free(info);
    return NULL;
}

static struct rtattr *net_rta_copy(struct rtattr *rta)
{
    struct rtattr *copy;

    copy = calloc(1, rta->rta_len);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, rta, rta->rta_len);

    return copy;
}

void net_info_free(struct net_info *info)
{
    int i;

    if (info == NULL) {
        return;
    }

    for (i = 0; i < IFLA_MAX; i++) {
        if (info->atts[i] == NULL) {
            continue;
        }
        free(info->atts[i]);
    }

    free(info);
}

int net_set_master(char *iface, char *master)
{
    int if_idx, br_idx;

    if_idx = net_ifindex(iface);
    if (if_idx < 0) {
        return -1;
    }

    br_idx = net_ifindex(master);
    if (br_idx < 0) {
        return -1;
    }

    return rt_link_set_attribute(if_idx, IFLA_MASTER, &br_idx, sizeof br_idx); 
}

int net_rename(char *old, char *new)
{
    int index;

    if (new == NULL || strlen(new) >= IF_NAMESIZE) {
        errno = EINVAL;
        return -1;
    }

    index = net_ifindex(old);
    if (index < 0) {
        return -1;
    }

    return rt_link_set_attribute(index, IFLA_IFNAME, new, strlen(new));
}

int net_add_ipv4(char *ifname, char *addr, unsigned char prefix)
{
    int index;
    struct in_addr addr_buf;
    struct in_addr bcast_buf;
    struct rt_encoder *enc;
    int err = -1;

    index = net_ifindex(ifname);
    if (index < 0) {
        return -1;
    }

    if (inet_pton(AF_INET, addr, &addr_buf) < 0) {
        return -1;
    }

    bcast_buf.s_addr = NET_IPV4_BCAST(addr_buf, prefix);

    enc = net_ipv4_req(index, &addr_buf, &bcast_buf, prefix);
    if (enc == NULL) {
        return -1;
    }

    err = rt_addr_add(enc->buf, enc->len);
    rt_enc_free(enc);
    return err;
}

static struct rt_encoder *net_ipv4_req(int index, const void *addr,
        const void *bcast, unsigned char prefix)
{
    struct rt_encoder *enc;
    struct ifaddrmsg ifa;

    enc = rt_enc_create();
    if (enc == NULL) {
        return NULL;
    }

    memset(&ifa, 0, sizeof ifa);
    ifa.ifa_family = AF_INET;
    ifa.ifa_index = index;
    ifa.ifa_prefixlen = prefix;

    if (rt_enc_data(enc, &ifa, sizeof ifa) < 0) {
        goto fail;
    }
    if (rt_enc_attribute(enc, IFA_ADDRESS, addr, NET_IPV4_LEN) < 0) {
        goto fail;
    }
    if (rt_enc_attribute(enc, IFA_LOCAL, addr, NET_IPV4_LEN) < 0) {
        goto fail;
    }
    if (rt_enc_attribute(enc, IFA_BROADCAST, bcast, NET_IPV4_LEN) < 0) {
        goto fail;
    }

    return enc;

fail:
    rt_enc_free(enc);
    return NULL;
}

int net_create_bridge(const char *name)
{
    int err = -1;
    struct ifinfomsg info;
    struct rt_encoder *enc;
    struct rt_encoder *linfo;

    if (name == NULL || strlen(name) >= IF_NAMESIZE) {
        errno = EINVAL;
        return -1;
    }

    linfo = rt_enc_create();
    if (linfo == NULL) {
        return -1;
    }
    if (rt_enc_attribute(linfo, IFLA_INFO_KIND, NET_LINK_BRIDGE,
                strlen(NET_LINK_BRIDGE)) < 0) {
        goto err_free_linfo;
    }

    memset(&info, 0, sizeof info);
    info.ifi_family = AF_UNSPEC;
    info.ifi_change = 0xFFFFFFFF;
    info.ifi_flags |= IFF_MULTICAST;

    enc = rt_enc_create();
    if (enc == NULL) {
        goto err_free_linfo;
    }
    if (rt_enc_data(enc, &info, sizeof info) < 0) {
        goto err_free_enc;
    }
    if (rt_enc_attribute(enc, IFLA_IFNAME, name, strlen(name)) < 0) {
        goto err_free_enc;
    }
    if (rt_enc_attribute(enc, IFLA_LINKINFO, linfo->buf, linfo->len) < 0) {
        goto err_free_enc;
    }

    err = rt_link_create(enc->buf, enc->len); 

err_free_enc:
    rt_enc_free(enc);
err_free_linfo:
    rt_enc_free(linfo);
    return err;
}


int net_create_veth(const char *name, const char *peer_name)
{
    int err = -1;
    struct ifinfomsg info;
    struct rt_encoder *enc;
    struct rt_encoder *linfo;
    struct rt_encoder *idata;
    struct rt_encoder *pinfo;

    if (name == NULL || peer_name == NULL ||
            strlen(name) >= IF_NAMESIZE || strlen(peer_name) >= IF_NAMESIZE) {
        errno = EINVAL;
        return -1;
    }

    // Used to configure both veth endpoints
    memset(&info, 0, sizeof info);
    info.ifi_family = AF_UNSPEC;
    info.ifi_change = 0xFFFFFFFF;
    info.ifi_flags |= IFF_MULTICAST;

    pinfo = rt_enc_create();
    if (pinfo == NULL) {
        return -1;
    }
    if (rt_enc_data(pinfo, &info, sizeof info) < 0) {
        goto err_free_pinfo;
    }
    if (rt_enc_attribute(pinfo, IFLA_IFNAME, peer_name,
                strlen(peer_name)) < 0) {
        goto err_free_pinfo;
    }

    idata = rt_enc_create();
    if (idata == NULL) {
        goto err_free_pinfo;
    }
    if (rt_enc_attribute(idata, VETH_INFO_PEER, pinfo->buf, pinfo->len) < 0) {
        goto err_free_idata;
    }

    linfo = rt_enc_create();
    if (linfo == NULL) {
        goto err_free_idata;
    }
    if (rt_enc_attribute(linfo, IFLA_INFO_KIND, NET_LINK_VETH,
                strlen(NET_LINK_VETH)) < 0) {
        goto err_free_linfo;
    }
    if (rt_enc_attribute(linfo, IFLA_INFO_DATA, idata->buf, idata->len) < 0) {
        goto err_free_linfo;
    }

    enc = rt_enc_create();
    if (enc == NULL) {
        goto err_free_linfo;
    }
    if (rt_enc_data(enc, &info, sizeof info) < 0) {
        goto err_free_enc;
    }
    if (rt_enc_attribute(enc, IFLA_IFNAME, name, strlen(name)) < 0) {
        goto err_free_enc;
    }
    if (rt_enc_attribute(enc, IFLA_LINKINFO, linfo->buf, linfo->len) < 0) {
        goto err_free_enc;
    }

    err = rt_link_create(enc->buf, enc->len);

err_free_enc:
    rt_enc_free(enc);
err_free_linfo:
    rt_enc_free(linfo);
err_free_idata:
    rt_enc_free(idata);
err_free_pinfo:
    rt_enc_free(pinfo);
    return err;
}

int net_delete(char *ifname)
{
    int i; 
    i = net_ifindex(ifname);
    if (i < 0) {
        return -1;
    }

    return rt_link_delete(i);
}

int net_up(char *ifname)
{
    return net_set_flags(ifname, IFF_UP, 0);
}

int net_down(char *ifname)
{
    return net_set_flags(ifname, 0, IFF_UP);
}

/**
 * net_set_flags changes the flags of the interface passed as parameter.
 *
 * @return  0 on success, -1 on failure
 */
static int net_set_flags(char *ifname, uint32_t set, uint32_t unset)
{
    int i;
    uint32_t flags;
    struct ifinfomsg *info;
    ssize_t info_len;

    i = net_ifindex(ifname);
    if (i < 0) {
        return -1;
    }

    info = calloc(RT_DGRAM_SIZE, 1);
    if (info == NULL) {
        return -1;
    }

    info_len = rt_link_info(i, info, RT_DGRAM_SIZE);
    if (info_len < 0) {
        goto free_buffer;
    }

    flags = info->ifi_flags;
    flags |= set;
    flags &= ~unset;

    if (rt_link_set_flags(i, flags) < 0) {
        goto free_buffer;
    }

    return 0;

free_buffer:
    free(info);
    return -1;
}

int net_is_up(char *ifname)
{
    int i;
    struct ifinfomsg *info;
    ssize_t info_len;

    i = net_ifindex(ifname);
    if (i < 0) {
        return -1;
    }

    info = calloc(RT_DGRAM_SIZE, 1);
    if (info == NULL) {
        return -1;
    }

    info_len = rt_link_info(i, info, RT_DGRAM_SIZE);
    if (info_len < 0) {
        free(info);
        return -1;
    }

    return info->ifi_flags & IFF_UP;
}

int net_ifindex(const char *ifname)
{
    if (!net_check_ifname(ifname)) {
        errno = EINVAL;
        return -1;
    } 

    return if_nametoindex(ifname);
}

char *net_ifname(int index, char *name)
{
    if (index < 0) {
        errno = EINVAL;
        return NULL;
    }

    return if_indextoname(index, name);
}

/**
 * net_check_ifname checks if the string passed as parameter is a valid
 * interface name.
 */
static bool net_check_ifname(const char *ifname)
{
    int l;

    if (ifname == NULL) {
        return false;
    }

    l = strlen(ifname);
    if (l == 0 || l >= IF_NAMESIZE) {
        return false;
    }

    return true;
}
