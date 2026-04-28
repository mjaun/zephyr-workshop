#define DT_DRV_COMPAT buzzer

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(buzzer);

struct buzzer_config {
    const struct device* pwm;
    const uint32_t channel;
    const pwm_flags_t flags;
};

static int buzzer_init(const struct device *dev)
{
    const struct buzzer_config *config = dev->config;

    if (!device_is_ready(config->pwm)) {
        LOG_ERR("device_is_ready");
        return -ENODEV;
    }

    return 0;
}

int buzzer_enable(const struct device *dev, uint32_t freq)
{
    const struct buzzer_config *config = dev->config;
    int ret;

    const uint32_t period = PWM_HZ(freq);
    const uint32_t pulse = period / 2;

    ret = pwm_set(config->pwm, config->channel, period, pulse, config->flags);

    if (ret < 0) {
        LOG_ERR("pwm_set: %d", ret);
        return ret;
    }

    return 0;
}

int buzzer_disable(const struct device *dev)
{
    const struct buzzer_config *config = dev->config;
    int ret;

    ret = pwm_set(config->pwm, config->channel, 0, 0, config->flags);

    if (ret < 0) {
        LOG_ERR("pwm_set: %d", ret);
        return ret;
    }

    return 0;
}

#define BUZZER_INIT(inst)                                                                          \
    static const struct buzzer_config buzzer_config_##inst = {                                     \
        .pwm = DEVICE_DT_GET(DT_INST_PHANDLE(inst, pwm)),                                          \
        .channel = DT_INST_PROP(inst, channel),                                                    \
        .flags = DT_INST_PROP(inst, flags),                                                        \
    };                                                                                             \
                                                                                                   \
    DEVICE_DT_INST_DEFINE(inst, buzzer_init, NULL /* power management */, NULL /* data */,         \
                          &buzzer_config_##inst, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,  \
                          NULL /* api */);

DT_INST_FOREACH_STATUS_OKAY(BUZZER_INIT)
