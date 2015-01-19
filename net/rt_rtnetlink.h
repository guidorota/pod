#ifndef _RT_NETLINK_H
#define _RT_NETLINK_H

#include <stdint.h>
#include <unistd.h>
#include <linux/rtnetlink.h>

/**
 * struct rt_ifinfo contains various interface information
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

/**
 * rt_set_flags sets the flags on an interface.
 *
 * @return  0 on success, -1 on failure
 */
int rt_set_flags(int index, uint32_t flags);

/**
 * rt_delete removes an interface from the system.
 *
 * @return  0 on success, -1 on failure
 */
int rt_delete(int index);

#endif /* _RT_NETLINK_H */
