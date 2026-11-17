#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/pwm/pwm_fake.h>
#include <zephyr/fff.h>
#include <zephyr/ztest.h>

#include "buzzer.h"

DEFINE_FFF_GLOBALS;

/*
 * fake_pwm_set_cycles is reset before every test by a ZTEST_RULE in the
 * fake PWM driver, so no manual RESET_FAKE() is needed here.
 */

#define BUZZER_NODE  DT_NODELABEL(buzzer)
#define FAKE_PWM_NODE DT_NODELABEL(fake_pwm)

static const struct device *const buzzer = DEVICE_DT_GET(BUZZER_NODE);
static const struct device *const fake_pwm = DEVICE_DT_GET(FAKE_PWM_NODE);

#define EXPECTED_CHANNEL DT_PROP(BUZZER_NODE, channel)
#define EXPECTED_FLAGS   DT_PROP(BUZZER_NODE, flags)

/* fake PWM runs at 1 GHz, so cycles equal nanoseconds */
BUILD_ASSERT(DT_PROP(FAKE_PWM_NODE, frequency) == NSEC_PER_SEC);

ZTEST(buzzer_suite, test_device_is_ready)
{
	zassert_true(device_is_ready(buzzer));
}

ZTEST(buzzer_suite, test_enable_sets_period_and_half_duty_cycle)
{
	zassert_ok(buzzer_enable(buzzer, 1000));

	zassert_equal(fake_pwm_set_cycles_fake.call_count, 1);
	zassert_equal(fake_pwm_set_cycles_fake.arg0_val, fake_pwm);
	zassert_equal(fake_pwm_set_cycles_fake.arg1_val, EXPECTED_CHANNEL);
	zassert_equal(fake_pwm_set_cycles_fake.arg2_val, 1000000); /* period: 1 ms */
	zassert_equal(fake_pwm_set_cycles_fake.arg3_val, 500000);  /* pulse: 50 % */
	zassert_equal(fake_pwm_set_cycles_fake.arg4_val, EXPECTED_FLAGS);
}

ZTEST(buzzer_suite, test_enable_with_other_frequency)
{
	zassert_ok(buzzer_enable(buzzer, 2500));

	zassert_equal(fake_pwm_set_cycles_fake.call_count, 1);
	zassert_equal(fake_pwm_set_cycles_fake.arg2_val, 400000);
	zassert_equal(fake_pwm_set_cycles_fake.arg3_val, 200000);
}

ZTEST(buzzer_suite, test_disable_sets_zero_period_and_pulse)
{
	zassert_ok(buzzer_disable(buzzer));

	zassert_equal(fake_pwm_set_cycles_fake.call_count, 1);
	zassert_equal(fake_pwm_set_cycles_fake.arg0_val, fake_pwm);
	zassert_equal(fake_pwm_set_cycles_fake.arg1_val, EXPECTED_CHANNEL);
	zassert_equal(fake_pwm_set_cycles_fake.arg2_val, 0);
	zassert_equal(fake_pwm_set_cycles_fake.arg3_val, 0);
	zassert_equal(fake_pwm_set_cycles_fake.arg4_val, EXPECTED_FLAGS);
}

ZTEST(buzzer_suite, test_enable_propagates_pwm_error)
{
	fake_pwm_set_cycles_fake.return_val = -EIO;

	zassert_equal(buzzer_enable(buzzer, 1000), -EIO);
	zassert_equal(fake_pwm_set_cycles_fake.call_count, 1);
}

ZTEST(buzzer_suite, test_disable_propagates_pwm_error)
{
	fake_pwm_set_cycles_fake.return_val = -EIO;

	zassert_equal(buzzer_disable(buzzer), -EIO);
	zassert_equal(fake_pwm_set_cycles_fake.call_count, 1);
}

ZTEST_SUITE(buzzer_suite, NULL, NULL, NULL, NULL, NULL);
