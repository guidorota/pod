#ifndef _RT_NETLINK_H
#define _RT_NETLINK_H

#include <unistd.h>
#include <linux/rtnetlink.h>

/**
 * Network interface information
 */
struct rt_ifinfo {
    struct ifinfomsg info;
    struct rtattr *atts[IFLA_MAX];
};

/**
 * rt_ifinfo_free releases the memory occupied by a struct rt_ifinfo.
 */
void rt_ifinfo_free(struct rt_ifinfo *ifinfo);

/**
 * rt_get_ifinfo collects information about the interface whose index is passed
 * as parameter.
 *
 * @return  struct ifinfomsg (refer to man 7 rtnetlink for parsing). This
 *          structure must be deallocated with free().
 */
struct rt_ifinfo *rt_get_ifinfo(int index);

#endif /* _RT_NETLINK_H */
