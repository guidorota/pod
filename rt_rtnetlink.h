#ifndef _RT_NETLINK_H
#define _RT_NETLINK_H

#include <linux/rtnetlink.h>

int rt_get_ifinfo(int index, struct ifinfomsg *info);

#endif /* _RT_NETLINK_H */
