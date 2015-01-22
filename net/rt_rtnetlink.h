#ifndef _RT_NETLINK_H
#define _RT_NETLINK_H

#include <stdint.h>
#include <unistd.h>
#include <linux/rtnetlink.h>

#define RT_MAX_ATTS IFLA_MAX

/**
 * struct rt_ifinfo contains interface information and attributes.
 */
struct rt_ifinfo; 

/**
 * struct rt_attlist is a list of rtattr data structures addressable by
 * attribute type.
 */
struct rt_attlist;

/**
 * rt_ifinfo_create instantiates an empty rt_ifinfo structure.
 */
struct rt_ifinfo *rt_ifinfo_create();

struct ifinfomsg *rt_ifinfo_get_ifinfomsg(struct rt_ifinfo *info);

/**
 * rt_attlist_create instantiates a new rt_attlist structure.
 */
struct rt_attlist *rt_attlist_create();

/**
 * rt_ifinfo_encode encodes a struct rt_ifinfo into the buffer passed as
 * parameter.
 *
 * @buf destination buffer
 * @len length of the destination buffer
 *
 * @return  number of bytes copied into the buffer, -1 if the buffer supplied
 * is not larg enough to contain all non-NULL elements in the atts array.
 */
ssize_t rt_ifinfo_encode(struct rt_ifinfo *info, void *buf, size_t len);

/**
 * rt_rtattr_encode encodes the non-NULL attributes present in the atts
 * parameter into the buffer passed as parameter.
 *
 * The user must guarantee that the atts parameter contains RT_MAX_ATTS
 * attributes.
 *
 * @buf destination buffer
 * @len length of the destination buffer
 *
 * @return  number of bytes copied into the buffer, -1 if the buffer supplied
 * is not large enough to contain all non-NULL elements in the atts array.
 */
ssize_t rt_attlist_encode(struct rt_attlist *atts, void *buf, size_t len);

/**
 * rt_attlist_add copies a new rtnetlink attribute to the array passed as
 * parameter.
 *
 * @return  0 on success, -1 failure
 */
int rt_attlist_add(struct rt_attlist *atts, unsigned short type,
        const void *buf, size_t len);

/**
 * rt_ifinfo_free releases the memory occupied by a struct rt_ifinfo.
 */
void rt_ifinfo_free(struct rt_ifinfo *ifinfo);

/**
 * rt_attlist_free releases the memory occupied by a struct rt_attlist.
 */
void rt_attlist_free(struct rt_attlist *atts);

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
