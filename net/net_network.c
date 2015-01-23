#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "net_network.h"
#include "rt_rtnetlink.h"

#define NET_LINK_VETH "veth"

static int net_set_flags(char *ifname, uint32_t set, uint32_t unset);
static bool net_check_ifname(const char *ifname);
static int net_ifindex(const char *ifname);

int net_create_veth(const char *name, const char *peer_name)
{
    int err = -1;
    struct ifinfomsg info;
    struct rt_encoder *enc;
    struct rt_encoder *linfo;
    struct rt_encoder *pinfo;

    if (name == NULL || peer_name == NULL ||
            strlen(name) > IF_NAMESIZE || strlen(peer_name) > IF_NAMESIZE) {
        errno = EINVAL;
        return -1;
    }

    // Used to configure both veth endpoints
    memset(&info, 0, sizeof info);
    info.ifi_family = AF_UNSPEC;
    info.ifi_change = 0xFFFFFFFF;
    info.ifi_flags |= IFF_UP;

    pinfo = rt_enc_create();
    if (pinfo == NULL) {
        return -1;
    }
    if (rt_enc_ifinfomsg(pinfo, &info) < 0) {
        goto err_free_pinfo;
    }
    if (rt_enc_attribute(pinfo, IFLA_IFNAME, peer_name,
                strlen(peer_name)) < 0) {
        goto err_free_pinfo;
    }

    linfo = rt_enc_create();
    if (linfo == NULL) {
        goto err_free_enc;
    }
    if (rt_enc_attribute(linfo, IFLA_INFO_KIND, NET_LINK_VETH,
                strlen(NET_LINK_VETH)) < 0) {
        goto err_free_linfo;
    }
    if (rt_enc_attribute(linfo, IFLA_INFO_DATA, pinfo->buf, pinfo->len) < 0) {
        goto err_free_linfo;
    }

    enc = rt_enc_create();
    if (enc == NULL) {
        goto err_free_linfo;
    }
    if (rt_enc_ifinfomsg(enc, &info) < 0) {
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
    int up;
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

/**
 * net_ifindex returns the index of the interface whose name is passed as
 * parameter.
 *
 * @return  < 0 in case of error
 */
static int net_ifindex(const char *ifname)
{
    if (!net_check_ifname(ifname)) {
        return -1;
    } 

    return if_nametoindex(ifname);
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
