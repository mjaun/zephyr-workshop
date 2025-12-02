#include "buzzer_ctrl.h"
#include "drivers/buzzer.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(buzzer_ctrl);

static void buzzer_thread_function(void *p1, void *p2, void *p3);

K_MSGQ_DEFINE(buzzer_msgq, sizeof(int), 3, 1);
K_THREAD_DEFINE(buzzer_thread, 1024, buzzer_thread_function, NULL, NULL, NULL, 5, 0, 0);

void buzzer_ctrl_post(const struct sensor_value *accel_z)
{
    int ret = k_msgq_put(&buzzer_msgq, accel_z, K_NO_WAIT);

    if (ret != 0) {
        LOG_WRN("k_msgq_put: %d", ret);
    }
}

static void buzzer_thread_function(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    const struct device *buzzer_dev = DEVICE_DT_GET(DT_CHOSEN(workshop_buzzer));
    struct sensor_value accel_z;
    int ret = 0;

    while (true) {
        ret = k_msgq_get(&buzzer_msgq, &accel_z, K_FOREVER);

        if (ret != 0) {
            LOG_ERR("k_msgq_get: %d", ret);
            continue;
        }

        buzzer_enable(buzzer_dev, accel_z.val1 < 9);
    }
}
