#ifndef _RT_NETLINK_H
#define _RT_NETLINK_H

#include <stdint.h>
#include <unistd.h>
#include <linux/rtnetlink.h>

#define RT_MAX_ATTS IFLA_MAX

/**
 * struct rt_ifinfo contains various interface information.
 */
struct rt_ifinfo {
    struct ifinfomsg info;
    struct rtattr **atts;
};

/**
 * rt_encode_ifinfomsg encodes a struct rt_ifinfo into the buffer passed as
 * parameter.
 *
 * @buf destination buffer
 * @len length of the destination buffer
 *
 * @return  number of bytes copied into the buffer, -1 if the buffer supplied
 * is not larg enough to contain all non-NULL elements in the atts array.
 */
ssize_t rt_encode_ifinfomsg(struct rt_ifinfo *info, void *buf, size_t len);

/**
 * rt_encode_rtattr encodes the non-NULL attributes present in the atts
 * parameter into the buffer passed as parameter.
 *
 * The user must guarantee that the **atts parameter contains RT_MAX_ATTS
 * attributes.
 *
 * @buf destination buffer
 * @len length of the destination buffer
 *
 * @return  number of bytes copied into the buffer, -1 if the buffer supplied
 * is not large enough to contain all non-NULL elements in the atts array.
 */
ssize_t rt_encode_rtattr(struct rtattr **atts, void *buf, size_t len);

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
