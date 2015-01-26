# Notes #

## network configuration ##

Root namespace:

    - bridge
    - one end of the veth pair assigned to the bridge
    - create a route to send all traffic for the namespace towards the bridge 
    - create a NAT rule inside iptables (masquerade) to allow the container to
      communicate with external networks

Guest namespace:

    - one end of the veth pair with address in the same network as the bridge
    - loopback interface has to be brought up and assigned the 127.0.0.1
      address
    - create a default route to route all unknown traffic through the veth
      endpoint available in the namespace

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
