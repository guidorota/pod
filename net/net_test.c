#include "net_network.h"
#include "net_test.h"

#define NET_VETH0 "tveth0"
#define NET_VETH1 "tveth1"

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
    int up0, up1;

    err = net_create_veth(NET_VETH0, NET_VETH1);
    ck_assert_int_ge(err, 0);

    up0 = net_is_up(NET_VETH0); 
    up1 = net_is_up(NET_VETH1);

    err = net_delete("tveth0");
    ck_assert_int_ge(err, 0);

    // postpone this check so that an error on net_is_up won't keep the test
    // case from deleting the veth pair
    ck_assert_int_ge(up0, 0);
    ck_assert_int_ge(up1, 0);
}
END_TEST

Suite *net_test_suite()
{
    Suite *s;
    TCase *c;

    s = suite_create("net");
    
    c = tcase_create("test_is_up");
    tcase_add_test(c, test_is_up);
    suite_add_tcase(s, c);

    c = tcase_create("test_create_delete");
    tcase_add_test(c, test_create_delete);
    suite_add_tcase(s, c);

    return s;
}
