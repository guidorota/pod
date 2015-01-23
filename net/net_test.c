#include "net_network.h"
#include "net_test.h"

START_TEST(test_is_up)
{
    int up;
    up = net_is_up("lo");

    ck_assert_int_eq(up, 1);
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

    return s;
}
