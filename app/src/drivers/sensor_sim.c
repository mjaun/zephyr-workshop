#define DT_DRV_COMPAT sensor_sim

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <stdlib.h>

struct sensor_sim_config {
    const char* state_file;
};

struct sensor_sim_data {
    struct sensor_value val;
};

static int sensor_sim_init(const struct device *dev)
{
    // nothing to do
    return 0;
}

static int sensor_sim_sample_fetch(const struct device *dev,
				enum sensor_channel chan)
{
    const struct sensor_sim_config *config = dev->config;
    struct sensor_sim_data *data = dev->data;

    FILE *f = fopen(config->state_file, "r");
    float value = 0;

    if (f != NULL) {
        if (fscanf(f, "%f", &value) > 0) {
            sensor_value_from_float(&data->val, value);
        } else {
            const struct sensor_value zero = {0};
            data->val = zero;
        }

        fclose(f);
    }

    return 0;
}

static int sensor_sim_channel_get(const struct device *dev,
			       enum sensor_channel chan,
			       struct sensor_value *val)
{
    struct sensor_sim_data *data = dev->data;

    switch (chan) {
	case SENSOR_CHAN_ACCEL_X:
	case SENSOR_CHAN_ACCEL_Y:
	case SENSOR_CHAN_ACCEL_Z:
	case SENSOR_CHAN_ACCEL_XYZ:
        *val = data->val;
        break;

    default:
        return -ENOTSUP;
    }

    return 0;
}

static DEVICE_API(sensor, sensor_sim_driver_api) = {
	.sample_fetch = sensor_sim_sample_fetch,
	.channel_get = sensor_sim_channel_get,
};

#define SENSOR_SIM_INIT(inst)                                                                                 \
    static const struct sensor_sim_config sensor_sim_config_##inst = {                                        \
        .state_file = DT_INST_PROP(inst, state_file),                                                         \
    };                                                                                                        \
                                                                                                              \
    static struct sensor_sim_data sensor_sim_data_##inst = {                                                  \
        .val = 0,                                                                                             \
    };                                                                                                        \
                                                                                                              \
    SENSOR_DEVICE_DT_INST_DEFINE(inst, sensor_sim_init, NULL /* power management */, &sensor_sim_data_##inst, \
                                 &sensor_sim_config_##inst, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,  \
                                 &sensor_sim_driver_api);

DT_INST_FOREACH_STATUS_OKAY(SENSOR_SIM_INIT)
