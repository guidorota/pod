#ifndef _RT_NETLINK_H
#define _RT_NETLINK_H

#include <stdint.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

/**
 * RT_DGRAM_SIZE is the typical size of a rtnetlink datagram message.
 */
#define RT_DGRAM_SIZE sysconf(_SC_PAGESIZE)

/**
 * RT_IFINFO_ALEN is the aligned length of a struct ifinfomsg.
 */
#define RT_IFINFO_ALEN NLMSG_ALIGN(sizeof (struct ifinfomsg))

/**
 * RT_RTA_LEN computes the total length of the rtattr attributes available
 * after a struct ifinfomsg.
 */
#define RT_RTA_LEN(len) (len - RT_IFINFO_ALEN)

/**
 * RT_RTA returns the first rtattr after a struct ifinfomsg.
 */
#define RT_RTA(buf) (struct rtattr *) ((char *) buf + RT_IFINFO_ALEN)

/**
 * struct rt_encoder is the supporting data strucure used to encode data into
 * appropriate rtnetlink messages.
 */
struct rt_encoder {
    void *buf;
    size_t cap;
    size_t len;
};

/**
 * rt_enc_create creates a new rtnetlink encoder whose maximum capacity is
 * equal to the page size in use by the kernel.
 *
 * @return  NULL on failure
 */
struct rt_encoder *rt_enc_create();

/**
 * rt_enc_create_cap creates a new rtnetlink encoder with the desired capacity.
 *
 * @cap maximum capacity
 *
 * @return NULL on failure
 */
struct rt_encoder *rt_enc_create_cap(size_t cap);

/**
 * rt_enc_data copies the content of a data buffer into the encoder.
*
 * @return 0 on successful encoding, -1 on failure
 */
int rt_enc_data(struct rt_encoder *e, const void *buf, size_t len);

/**
 * rt_enc_attribute encodes a struct rtattr.
 *
 * @return 0 on successful encoding, -1 on failure
 */
int rt_enc_attribute(struct rt_encoder *e, unsigned int type,
        const void *buf, size_t len);

/**
 * rt_enc_free frees the memory occupied by the encoder passed as parameter.
 */
void rt_enc_free(struct rt_encoder *e);

/**
 * rt_addr_add adds an address to an interface.
 *
 * @return  -1 in case of error
 */
int rt_addr_add(const struct ifaddrmsg *ifa, size_t ifa_len);

/**
 * rt_link_create creates a new link.
 *
 * @return  0 on success, -1 on failure
 */
int rt_link_create(const struct ifinfomsg *info, size_t info_len);

/**
 * rt_link_set_attribute sets an interface attribute
 *
 * @return  0 on success, -1 on failure
 */
int rt_link_set_attribute(int index, unsigned int attr, const void *buf,
        size_t len);

/**
 * rt_link_delete removes an interface from the system.
 *
 * @return  0 on success, -1 on failure
 */
int rt_link_delete(int index);

/**
 * rt_link_set_flags sets the flags on an interface.
 *
 * @return  0 on success, -1 on failure
 */
int rt_link_set_flags(int index, uint32_t flags);

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

#endif /* _RT_NETLINK_H */
