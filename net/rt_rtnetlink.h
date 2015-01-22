#ifndef _RT_NETLINK_H
#define _RT_NETLINK_H

#include <stdint.h>
#include <unistd.h>
#include <linux/rtnetlink.h>

/**
 * Typical size of a rtnetlink datagram message.
 */
#define RT_DGRAM_SIZE sysconf(_SC_PAGESIZE)

/**
 * rt_link_create creates a new link.
 *
 * @return  0 on success, -1 on failure
 */
int rt_link_create(struct ifinfomsg *info, size_t info_len);

/**
 * rt_link_info collects information about the interface whose index is passed
 * as parameter.
 *
 * The contents of the ifinfomsg structure are copied inside the buffer passed
 * as parameter.
 *
 * @return  length of the struct ifinfomsg copied in the buffer passed as
 *          parameter, -1 on failure. This function will set errno to EOVERFLOW
 *          if the buffer is not large enough to store all the link information
 *          returned by the kernel.
 */
ssize_t rt_link_info(int index, void *buf, size_t len);

/**
 * rt_delete_link removes an interface from the system.
 *
 * @return  0 on success, -1 on failure
 */
int rt_delete_link(int index);

/**
 * rt_set_flags sets the flags on an interface.
 *
 * @return  0 on success, -1 on failure
 */
int rt_set_link_flags(int index, uint32_t flags);

#endif /* _RT_NETLINK_H */
