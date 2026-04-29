#include <zephyr/kernel.h>
#include <gmock/gmock.h>
#include <vector>
#include "posix_board_if.h"
#include "nsi_cmdline.h"

extern "C" int main(void)
{
    int test_argc;
    char** test_argv;

    // returns all arguments after -testargs
    nsi_get_test_cmd_line_args(&test_argc, &test_argv);

    // prepend a dummy application name as argument 0
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>("zephyr_test"));
    argv.insert(argv.end(), test_argv, test_argv + test_argc);
    int argc = static_cast<int>(argv.size());

    testing::InitGoogleMock(&argc, argv.data());

    int ret = RUN_ALL_TESTS();

    posix_exit(ret);
    return 0;
}
