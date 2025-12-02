#define DT_DRV_COMPAT buzzer

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(buzzer);

struct buzzer_config {
    const struct pwm_dt_spec pwm;
    const uint32_t frequency;
};

static int buzzer_init(const struct device *dev)
{
    const struct buzzer_config *config = dev->config;

    if (!pwm_is_ready_dt(&config->pwm)) {
        LOG_ERR("pwm_is_ready_dt");
        return -ENODEV;
    }

    return 0;
}

int buzzer_enable(const struct device *dev, bool enable)
{
    const struct buzzer_config *config = dev->config;
    int ret;

    const uint32_t period = PWM_HZ(config->frequency);
    const uint32_t pulse = enable ? (period / 2) : 0;
    ret = pwm_set_dt(&config->pwm, period, pulse);

    if (ret < 0) {
        LOG_ERR("pwm_set_dt: %d", ret);
        return ret;
    }

    return 0;
}

#define BUZZER_INIT(inst)                                                                          \
    static const struct buzzer_config buzzer_config_##inst = {                                     \
        .pwm = PWM_DT_SPEC_INST_GET(inst),                                                         \
        .frequency = DT_INST_PROP(inst, frequency),                                                \
    };                                                                                             \
                                                                                                   \
    DEVICE_DT_INST_DEFINE(inst, buzzer_init, NULL /* power management */, NULL /* data */,         \
                          &buzzer_config_##inst, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,  \
                          NULL /* api */);

DT_INST_FOREACH_STATUS_OKAY(BUZZER_INIT)
