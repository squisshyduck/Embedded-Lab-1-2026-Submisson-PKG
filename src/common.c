#include "common.h"

static bool valid_mask(status_flags_t mask) {
    return mask != 0U && (mask & (status_flags_t)~STATUS_FLAG_ALL) == 0U;
}

bool status_set(status_flags_t *flags, status_flags_t mask) {
    if (flags == NULL || !valid_mask(mask)) {
        return false;
    }

    *flags = (status_flags_t)(*flags | mask);
    return true;
}

bool status_clear(status_flags_t *flags, status_flags_t mask) {
    if (flags == NULL || !valid_mask(mask)) {
        return false;
    }

    *flags = (status_flags_t)(*flags & (status_flags_t)~mask);
    return true;
}

bool status_toggle(status_flags_t *flags, status_flags_t mask) {
    if (flags == NULL || !valid_mask(mask)) {
        return false;
    }

    *flags = (status_flags_t)(*flags ^ mask);
    return true;
}

bool status_test(status_flags_t flags, status_flags_t mask) {
    return valid_mask(mask) && (flags & mask) == mask;
}

system_state_t state_update(system_state_t current, const sample_t *sample) {
    if (sample == NULL ||
        current < SYSTEM_STATE_INIT ||
        current > SYSTEM_STATE_FAULT ||
        !status_test(sample->status_bits, STATUS_FLAG_SENSOR_VALID)) {
        return SYSTEM_STATE_FAULT;
    }

    if (current == SYSTEM_STATE_FAULT) {
        return sample->value_x10 < STATE_FAULT_RECOVERY_X10
                   ? SYSTEM_STATE_NORMAL
                   : SYSTEM_STATE_FAULT;
    }

    if (sample->value_x10 >= STATE_FAULT_THRESHOLD_X10) {
        return SYSTEM_STATE_FAULT;
    }

    if (sample->value_x10 >= STATE_WARNING_THRESHOLD_X10) {
        return SYSTEM_STATE_WARNING;
    }

    return SYSTEM_STATE_NORMAL;
}
