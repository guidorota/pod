#ifndef _RT_NETLINK_H
#define _RT_NETLINK_H

#include <linux/rtnetlink.h>

/**
 * rt_get_ifinfo collects information about the interface whose index is passed
 * as parameter.
 *
 * @return  struct ifinfomsg (refer to man 7 rtnetlink for parsing). This
 *          structure must be deallocated with free().
 */
struct ifinfomsg *rt_get_ifinfo(int index);

#endif /* _RT_NETLINK_H */
