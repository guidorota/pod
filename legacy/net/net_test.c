#include "net_network.h"
#include "net_test.h"

#define NET_VETH0 "tveth0"
#define NET_VETH1 "tveth1"
#define NET_NEWNAME "newname"
#define NET_BRIDGE "tbridge"

START_TEST(test_name_index)
{
    char name_buf[IF_NAMESIZE];
    char *name;
    int index;

    index = net_ifindex("lo");
    ck_assert_int_ge(index, 0);

    name = net_ifname(index, name_buf);
    ck_assert_ptr_ne(name, NULL);
    ck_assert_str_eq(name, "lo");
}
END_TEST

START_TEST(test_info)
{
    struct net_info *info;
    struct rtattr *rta;

    info = net_info("lo");
   
    ck_assert_ptr_ne(NULL, info);
    rta = info->atts[IFLA_IFNAME];
    ck_assert_str_eq((char *) RTA_DATA(rta), "lo");

    net_info_free(info);
}
END_TEST

START_TEST(test_is_up)
{
    int up;
    up = net_is_up("lo");

    ck_assert_int_eq(up, 1);
}
END_TEST

START_TEST(test_create_delete)
{
    int err;
    struct net_info *info;
    struct rtattr *rta;

    err = net_create_veth(NET_VETH0, NET_VETH1);
    ck_assert_int_ge(err, 0);

    info = net_info(NET_VETH0);
    ck_assert_ptr_ne(info, NULL);
    rta = info->atts[IFLA_IFNAME];
    ck_assert_str_eq((char *) RTA_DATA(rta), NET_VETH0);

    info = net_info(NET_VETH1);
    ck_assert_ptr_ne(info, NULL); 
    rta = info->atts[IFLA_IFNAME];
    ck_assert_str_eq((char *) RTA_DATA(rta), NET_VETH1);

    err = net_delete(NET_VETH0);
    ck_assert_int_ge(err, 0);
}
END_TEST

START_TEST(test_up_down)
{
    int err;

    err = net_create_veth(NET_VETH0, NET_VETH1);
    ck_assert_int_ge(err, 0);

    ck_assert_int_eq(net_is_up(NET_VETH0), 0);
    ck_assert_int_ge(net_up(NET_VETH0), 0);
    ck_assert_int_eq(net_is_up(NET_VETH0), 1);
    ck_assert_int_eq(net_is_up(NET_VETH1), 0);

    ck_assert_int_ge(net_down(NET_VETH0), 0);
    ck_assert_int_eq(net_is_up(NET_VETH0), 0);
    ck_assert_int_eq(net_is_up(NET_VETH1), 0);

    ck_assert_int_ge(net_delete(NET_VETH0), 0);
}
END_TEST

START_TEST(test_rename)
{
    int err;
    struct net_info *info;
    struct rtattr *rta;

    err = net_create_veth(NET_VETH0, NET_VETH1);
    ck_assert_int_ge(err, 0);

    info = net_info(NET_VETH0);
    ck_assert_ptr_ne(info, NULL);

    rta = info->atts[IFLA_IFNAME];
    ck_assert_str_eq(RTA_DATA(rta), NET_VETH0);
    net_info_free(info);

    err = net_rename(NET_VETH1, NET_NEWNAME);
    ck_assert_int_ge(err, 0);

    info = net_info(NET_NEWNAME);
    ck_assert_ptr_ne(info, NULL);

    rta = info->atts[IFLA_IFNAME];
    ck_assert_str_eq(RTA_DATA(rta), NET_NEWNAME);
    net_info_free(info);

    ck_assert_int_ge(net_delete(NET_VETH0), 0);
}
END_TEST

START_TEST(test_bridge)
{
    int err;
    struct net_info *info;
    struct rtattr *rta;

    err = net_create_bridge(NET_BRIDGE);
    ck_assert_int_ge(err, 0);

    info = net_info(NET_BRIDGE);
    ck_assert_ptr_ne(info, NULL);
    
    rta = info->atts[IFLA_IFNAME];    
    ck_assert_str_eq(RTA_DATA(rta), NET_BRIDGE);
    net_info_free(info);

    err = net_delete(NET_BRIDGE);
    ck_assert_int_ge(err, 0);
}
END_TEST

START_TEST(test_set_master)
{
    int err;
    struct net_info *info;
    struct rtattr *rta;
    char name_buf[IF_NAMESIZE];
    char *name;

    err = net_create_bridge(NET_BRIDGE);
    ck_assert_int_ge(err, 0);

    err = net_create_veth(NET_VETH0, NET_VETH1);
    ck_assert_int_ge(err, 0);

    err = net_set_master(NET_VETH0, NET_BRIDGE);
    ck_assert_int_ge(err, 0);

    info = net_info(NET_VETH0);
    ck_assert_ptr_ne(info, NULL);
    rta = info->atts[IFLA_MASTER];
    ck_assert_ptr_ne(rta, NULL);

    name = net_ifname(*((int *) RTA_DATA(rta)), name_buf);
    ck_assert_str_eq(name, NET_BRIDGE);

    err = net_delete(NET_VETH0);
    ck_assert_int_ge(err, 0);
    err = net_delete(NET_BRIDGE);
    ck_assert_int_ge(err, 0);
}
END_TEST

Suite *net_test_suite()
{
    Suite *s;
    TCase *c;

    s = suite_create("net");

    c = tcase_create("test_name_index");
    tcase_add_test(c, test_name_index);
    suite_add_tcase(s, c);

    c = tcase_create("test_info");
    tcase_add_test(c, test_info);
    suite_add_tcase(s, c);
    
    c = tcase_create("test_is_up");
    tcase_add_test(c, test_is_up);
    suite_add_tcase(s, c);

    c = tcase_create("test_create_delete");
    tcase_add_test(c, test_create_delete);
    suite_add_tcase(s, c);

    c = tcase_create("test_up_down");
    tcase_add_test(c, test_up_down);
    suite_add_tcase(s, c);

    c = tcase_create("test_rename");
    tcase_add_test(c, test_rename);
    suite_add_tcase(s, c);
    
    c = tcase_create("test_bridge");
    tcase_add_test(c, test_bridge);
    suite_add_tcase(s, c);
    
    c = tcase_create("test_set_master");
    tcase_add_test(c, test_set_master);
    suite_add_tcase(s, c);

    return s;
}