#define DT_DRV_COMPAT pwm_sim

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/pwm.h>

LOG_MODULE_REGISTER(pwm_sim);

struct pwm_sim_data {
    uint32_t period;
    uint32_t pulse;
};

static int pwm_sim_init(const struct device *dev)
{
    return 0;
}

static int pwm_sim_set_cycles(const struct device *dev, uint32_t channel,
                               uint32_t period_cycles, uint32_t pulse_cycles,
                               pwm_flags_t flags)
{
    struct pwm_sim_data *data = dev->data;

    if (period_cycles != data->period || pulse_cycles != data->pulse) {
        data->period = period_cycles;
        data->pulse = pulse_cycles;
        LOG_INF("period: %u, pulse: %u", period_cycles, pulse_cycles);
    }

    return 0;
}

static int pwm_sim_get_cycles_per_sec(const struct device *dev, uint32_t channel,
                                       uint64_t *cycles)
{
    *cycles = NSEC_PER_SEC;
    return 0;
}

static DEVICE_API(pwm, pwm_sim_driver_api) = {
    .set_cycles = pwm_sim_set_cycles,
    .get_cycles_per_sec = pwm_sim_get_cycles_per_sec,
};

#define PWM_SIM_INIT(inst)                                                        \
    static struct pwm_sim_data pwm_sim_data_##inst = { .period = 0, .pulse = 0 }; \
                                                                                  \
    DEVICE_DT_INST_DEFINE(inst, pwm_sim_init, NULL /* power management */,        \
                          &pwm_sim_data_##inst, NULL /* config */,                \
                          POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,        \
                          &pwm_sim_driver_api);

DT_INST_FOREACH_STATUS_OKAY(PWM_SIM_INIT)
