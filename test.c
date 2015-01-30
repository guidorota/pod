#include <check.h>
#include <stdlib.h>

#include "utils/dy_dynamicbuffer_test.h"
#include "net/net_test.h"

int main(void)
{
    int failed;
    SRunner *sr;

    sr = srunner_create(NULL);
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_add_suite(sr, net_test_suite());
    srunner_add_suite(sr, dy_dynamicbuffer_test_suite());

    srunner_run_all(sr, CK_VERBOSE);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    if (failed != 0) {
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
