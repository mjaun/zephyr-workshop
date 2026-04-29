#include <zephyr/ztest.h>
#include "frequency.h"

ZTEST(frequency_suite, test_above_threshold_returns_zero)
{
    zassert_equal(accel_to_freq(9.0f), 0);
    zassert_equal(accel_to_freq(8.1f), 0);
}

ZTEST(frequency_suite, test_at_min_accel_returns_min_freq)
{
    zassert_equal(accel_to_freq(8.0f), 500);
}

ZTEST(frequency_suite, test_at_max_accel_returns_max_freq)
{
    zassert_equal(accel_to_freq(-9.0f), 3000);
}

ZTEST(frequency_suite, test_midpoint)
{
    /* accel = -0.5: 500 + (-0.5 - 8) * 2500 / -17 = 1750 */
    zassert_equal(accel_to_freq(-0.5f), 1750);
}

ZTEST_SUITE(frequency_suite, NULL, NULL, NULL, NULL, NULL);
