#ifndef EMBEDDED_COMMON_H
#define EMBEDDED_COMMON_H

#include <stdbool.h>
#include <stdint.h>

typedef uint16_t status_flags_t;

#define STATUS_FLAG_SENSOR_VALID UINT16_C(0x0001)
#define STATUS_FLAG_LINK_READY UINT16_C(0x0002)
#define STATUS_FLAG_SAMPLE_READY UINT16_C(0x0004)
#define STATUS_FLAG_OUTPUT_ACTIVE UINT16_C(0x0008)
#define STATUS_FLAG_WARNING UINT16_C(0x0010)
#define STATUS_FLAG_FAULT UINT16_C(0x0020)
#define STATUS_FLAG_CALIBRATED UINT16_C(0x0040)
#define STATUS_FLAG_MAINTENANCE UINT16_C(0x0080)
#define STATUS_FLAG_ALL UINT16_C(0x00ff)

typedef struct {
    uint32_t timestamp_ms;
    status_flags_t status_bits;
    int16_t value_x10;
} sample_t;

typedef struct {
    uint16_t node_id;
    status_flags_t status_bits;
    uint32_t sample_count;
} node_status_t;

typedef enum {
    SYSTEM_STATE_INIT = 0,
    SYSTEM_STATE_NORMAL,
    SYSTEM_STATE_WARNING,
    SYSTEM_STATE_FAULT
} system_state_t;

#define STATE_WARNING_THRESHOLD_X10 INT16_C(800)
#define STATE_FAULT_THRESHOLD_X10 INT16_C(1000)
#define STATE_FAULT_RECOVERY_X10 INT16_C(600)

bool status_set(status_flags_t *flags, status_flags_t mask);
bool status_clear(status_flags_t *flags, status_flags_t mask);
bool status_toggle(status_flags_t *flags, status_flags_t mask);
bool status_test(status_flags_t flags, status_flags_t mask);

system_state_t state_update(system_state_t current, const sample_t *sample);

#endif
