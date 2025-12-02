#include <zephyr/ztest.h>
#include <zephyr/fff.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include "sensor_fake.h"
#include "sensor_ctrl.h"

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(buzzer_ctrl_post, const struct sensor_value *);

static int fake_sensor_channel_get_success(const struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(chan);

    val->val1 = 9;
    val->val2 = 123456;

    return 0;
}

static void sensor_ctrl_test_setup(void *fixture)
{
    ARG_UNUSED(fixture);

    RESET_FAKE(fake_sensor_sample_fetch);
    RESET_FAKE(fake_sensor_channel_get);
    RESET_FAKE(buzzer_ctrl_post);
}

ZTEST_SUITE(sensor_ctrl_tests, NULL, NULL, sensor_ctrl_test_setup, NULL, NULL);

ZTEST(sensor_ctrl_tests, test_sensor_ctrl_success)
{
    // Arrange
    fake_sensor_sample_fetch_fake.return_val = 0;
    fake_sensor_channel_get_fake.custom_fake = fake_sensor_channel_get_success;

    sensor_ctrl_start();

    // Act
    k_sleep(K_USEC(1500));

    // Assert
    zassert_true(fake_sensor_sample_fetch_fake.call_count == 1, "should call sensor_sample_fetch");
    zassert_true(fake_sensor_channel_get_fake.call_count == 1, "should call sensor_channel_get");
    zassert_equal(fake_sensor_channel_get_fake.arg1_val, SENSOR_CHAN_ACCEL_Z, "should request ACCEL_Z channel");

    zassert_true(buzzer_ctrl_post_fake.call_count == 1, "should call buzzer_ctrl_post");

    const struct sensor_value *posted_value = buzzer_ctrl_post_fake.arg0_val;
    zassert_not_null(posted_value, "posted value should not be NULL");
    zassert_equal(posted_value->val1, 9, "posted value val1 should be 9");
    zassert_equal(posted_value->val2, 123456, "posted value val2 should be 123456");
}
