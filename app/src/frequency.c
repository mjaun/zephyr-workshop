#include "frequency.h"

#define MIN_FREQ   500.0f
#define MAX_FREQ   3000.0f
#define MIN_ACCEL  8.0f
#define MAX_ACCEL  -9.0f

uint32_t accel_to_freq(float accel)
{
    if (accel > MIN_ACCEL) {
        return 0;
    }

    float freq = MIN_FREQ + (accel - MIN_ACCEL) * (MAX_FREQ - MIN_FREQ) / (MAX_ACCEL - MIN_ACCEL);
    return (uint32_t)freq;
}
