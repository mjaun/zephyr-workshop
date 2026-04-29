#ifndef FREQUENCY_H
#define FREQUENCY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Map an acceleration value to a buzzer frequency in Hz.
 *
 * Returns 0 when the buzzer should be off (accel > 8.0).
 * Otherwise returns a value linearly interpolated in [500, 3000] Hz
 * for accel in [8.0, -9.0].
 */
uint32_t accel_to_freq(float accel);

#ifdef __cplusplus
}
#endif

#endif /* FREQUENCY_H */
