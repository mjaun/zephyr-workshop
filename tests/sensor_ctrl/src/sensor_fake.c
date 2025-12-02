#define DT_DRV_COMPAT fake_sensor

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/ztest.h>
#include <zephyr/fff.h>
#include "sensor_fake.h"

DEFINE_FAKE_VALUE_FUNC(int, fake_sensor_sample_fetch, const struct device *, enum sensor_channel);

DEFINE_FAKE_VALUE_FUNC(int, fake_sensor_channel_get, const struct device *, enum sensor_channel,
                       struct sensor_value *);

static const struct sensor_driver_api fake_sensor_driver_api = {
    .sample_fetch = fake_sensor_sample_fetch,
    .channel_get = fake_sensor_channel_get,
};

#define FAKE_SENSOR_INIT(inst)                                                                     \
    DEVICE_DT_INST_DEFINE(inst, NULL, /* init_fn */                                                \
                          NULL,       /* pm */                                                     \
                          NULL,       /* data */                                                   \
                          NULL,       /* config */                                                 \
                          POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &fake_sensor_driver_api);

DT_INST_FOREACH_STATUS_OKAY(FAKE_SENSOR_INIT)
