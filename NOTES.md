# Notes #

## veth creation ##

Main veth creation code resides in net/core/rtnetlink.c (rtnl_newlink) and
drivers/net/veth.c.

Attributes:

    - IFLA_IFNAME
    - IFLA_ADDRESS (optional)
    - IFLA_BROADCAST (optional) broadcast address
    - IFLA_AF_SPEC (optional)
    - IFLA_LINKINFO
        - IFLA_INFO_KIND string, defines the type of interface to create
        - IFLA_INFO_DATA
            - VETH_INFO_PEER
                - ifinfomsg
                - IFLA_IFNAME
