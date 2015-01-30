#include "dy_dynamicbuffer.h"
#include "dy_dynamicbuffer_test.h"

START_TEST(test_create)
{
    struct dy_dynamicbuffer *d;

    d = dy_create();
    ck_assert_ptr_ne(d, NULL);
    ck_assert_ptr_ne(d->buf, NULL);
    ck_assert_int_eq(d->len, 0);
    ck_assert_int_gt(d->cap, 0);

    dy_free(d);
}
END_TEST

START_TEST(test_create_cap)
{
    struct dy_dynamicbuffer *d;

    d = dy_create_cap(10);
    ck_assert_ptr_ne(d, NULL);
    ck_assert_ptr_ne(d->buf, NULL);

    ck_assert_int_eq(d->cap, 10);
    ck_assert_int_eq(d->len, 0);

    dy_free(d);
}
END_TEST

START_TEST(test_add)
{
    struct dy_dynamicbuffer *d;
    size_t cap_before;
    int value = 10;

    d = dy_create();
    ck_assert_ptr_ne(d, NULL);

    cap_before = d->cap;
    dy_add(d, &value, sizeof value);
    ck_assert_int_eq(d->cap, cap_before);
    ck_assert_int_eq(d->len, sizeof value);
    ck_assert_int_eq(*((int *) d->buf), value);

    dy_free(d);
}
END_TEST

START_TEST(test_expand)
{
    uint32_t value = 10;
    struct dy_dynamicbuffer *d;

    d = dy_create_cap(7);
    ck_assert_ptr_ne(d, NULL);

    dy_add(d, &value, sizeof value);
    ck_assert_int_eq(d->len, sizeof value);
    ck_assert_int_eq(d->cap, 7);

    dy_add(d, &value, sizeof value);
    ck_assert_int_eq(d->len, 2 * sizeof value);
    ck_assert_int_gt(d->cap, 7);

    dy_free(d);
}
END_TEST

Suite *dy_dynamicbuffer_test_suite()
{
    Suite *s;
    TCase *c;

    s = suite_create("dynamicbuffer");

    c = tcase_create("test_create");
    tcase_add_test(c, test_create);
    suite_add_tcase(s, c);

    c = tcase_create("test_create_cap");
    tcase_add_test(c, test_create_cap);
    suite_add_tcase(s, c);

    c = tcase_create("test_add");
    tcase_add_test(c, test_add);
    suite_add_tcase(s, c);

    c = tcase_create("test_expand");
    tcase_add_test(c, test_expand);
    suite_add_tcase(s, c);

    return s;
}
