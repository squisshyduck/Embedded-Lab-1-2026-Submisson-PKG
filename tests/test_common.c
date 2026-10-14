#include "common.h"

#include <stdio.h>
#include <string.h>

typedef bool (*test_fn_t)(void);

typedef struct {
    const char *name;
    test_fn_t run;
} test_case_t;

#define CHECK(condition)                     \
    do {                                      \
        if (!(condition)) {                   \
            return false;                    \
        }                                     \
    } while (0)

static bool test_set_and_test_flag(void) {
    status_flags_t flags = 0U;
    CHECK(status_set(&flags, STATUS_FLAG_SENSOR_VALID));
    CHECK(status_test(flags, STATUS_FLAG_SENSOR_VALID));
    return true;
}

static bool test_set_multiple_flags(void) {
    status_flags_t flags = 0U;
    CHECK(status_set(&flags, STATUS_FLAG_LINK_READY | STATUS_FLAG_OUTPUT_ACTIVE));
    CHECK(status_test(flags, STATUS_FLAG_LINK_READY | STATUS_FLAG_OUTPUT_ACTIVE));
    return true;
}

static bool test_clear_flag(void) {
    status_flags_t flags = STATUS_FLAG_WARNING | STATUS_FLAG_LINK_READY;
    CHECK(status_clear(&flags, STATUS_FLAG_WARNING));
    CHECK(!status_test(flags, STATUS_FLAG_WARNING));
    CHECK(status_test(flags, STATUS_FLAG_LINK_READY));
    return true;
}

static bool test_toggle_flag(void) {
    status_flags_t flags = 0U;
    CHECK(status_toggle(&flags, STATUS_FLAG_SAMPLE_READY));
    CHECK(status_test(flags, STATUS_FLAG_SAMPLE_READY));
    CHECK(status_toggle(&flags, STATUS_FLAG_SAMPLE_READY));
    CHECK(!status_test(flags, STATUS_FLAG_SAMPLE_READY));
    return true;
}

static bool test_reserved_flag_is_rejected(void) {
    status_flags_t flags = 0U;
    CHECK(!status_set(&flags, UINT16_C(0x0100)));
    CHECK(flags == 0U);
    return true;
}

static bool test_null_flag_pointer_is_rejected(void) {
    CHECK(!status_set(NULL, STATUS_FLAG_LINK_READY));
    CHECK(!status_clear(NULL, STATUS_FLAG_LINK_READY));
    CHECK(!status_toggle(NULL, STATUS_FLAG_LINK_READY));
    return true;
}

static bool test_normal_state(void) {
    const sample_t sample = { 1U, STATUS_FLAG_SENSOR_VALID, 500 };
    CHECK(state_update(SYSTEM_STATE_INIT, &sample) == SYSTEM_STATE_NORMAL);
    return true;
}

static bool test_warning_boundary(void) {
    const sample_t sample = { 2U, STATUS_FLAG_SENSOR_VALID, STATE_WARNING_THRESHOLD_X10 };
    CHECK(state_update(SYSTEM_STATE_NORMAL, &sample) == SYSTEM_STATE_WARNING);
    return true;
}

static bool test_fault_boundary(void) {
    const sample_t sample = { 3U, STATUS_FLAG_SENSOR_VALID, STATE_FAULT_THRESHOLD_X10 };
    CHECK(state_update(SYSTEM_STATE_NORMAL, &sample) == SYSTEM_STATE_FAULT);
    return true;
}

static bool test_invalid_sensor_enters_fault(void) {
    const sample_t sample = { 4U, 0U, 200 };
    CHECK(state_update(SYSTEM_STATE_NORMAL, &sample) == SYSTEM_STATE_FAULT);
    return true;
}

static bool test_fault_requires_recovery_boundary(void) {
    const sample_t at_boundary = {
        5U, STATUS_FLAG_SENSOR_VALID, STATE_FAULT_RECOVERY_X10
    };
    const sample_t below_boundary = {
        6U, STATUS_FLAG_SENSOR_VALID, STATE_FAULT_RECOVERY_X10 - 1
    };
    CHECK(state_update(SYSTEM_STATE_FAULT, &at_boundary) == SYSTEM_STATE_FAULT);
    CHECK(state_update(SYSTEM_STATE_FAULT, &below_boundary) == SYSTEM_STATE_NORMAL);
    return true;
}

static bool test_null_sample_enters_fault(void) {
    CHECK(state_update(SYSTEM_STATE_NORMAL, NULL) == SYSTEM_STATE_FAULT);
    return true;
}

static bool test_invalid_current_state_enters_fault(void) {
    const sample_t sample = { 7U, STATUS_FLAG_SENSOR_VALID, 200 };
    CHECK(state_update((system_state_t)99, &sample) == SYSTEM_STATE_FAULT);
    return true;
}

static int run_deliberate_failure_demo(void) {
    const sample_t sample = { 8U, STATUS_FLAG_SENSOR_VALID, 500 };
    const system_state_t actual = state_update(SYSTEM_STATE_INIT, &sample);
    const system_state_t intentionally_wrong_expectation = SYSTEM_STATE_WARNING;

    if (actual == intentionally_wrong_expectation) {
        (void)fprintf(stderr, "FAIL: deliberately incorrect expectation unexpectedly passed\n");
        return 0;
    }

    (void)printf("EXPECTED FAILURE: expected state %d, observed state %d\n",
                 (int)intentionally_wrong_expectation, (int)actual);
    return 1;
}

int main(int argc, char **argv) {
    static const test_case_t tests[] = {
        { "set and test flag", test_set_and_test_flag },
        { "set multiple flags", test_set_multiple_flags },
        { "clear flag", test_clear_flag },
        { "toggle flag", test_toggle_flag },
        { "reject reserved flag", test_reserved_flag_is_rejected },
        { "reject null flag pointer", test_null_flag_pointer_is_rejected },
        { "normal state", test_normal_state },
        { "warning boundary", test_warning_boundary },
        { "fault boundary", test_fault_boundary },
        { "invalid sensor enters fault", test_invalid_sensor_enters_fault },
        { "fault recovery boundary", test_fault_requires_recovery_boundary },
        { "null sample enters fault", test_null_sample_enters_fault },
        { "invalid current state enters fault", test_invalid_current_state_enters_fault }
    };
    const size_t test_count = sizeof(tests) / sizeof(tests[0]);
    size_t index;
    size_t passed = 0U;

    if (argc == 2 && strcmp(argv[1], "--demo-failure") == 0) {
        return run_deliberate_failure_demo();
    }

    if (argc != 1) {
        (void)fprintf(stderr, "usage: %s [--demo-failure]\n", argv[0]);
        return 2;
    }

    for (index = 0U; index < test_count; ++index) {
        const bool succeeded = tests[index].run();
        (void)printf("%s: %s\n", succeeded ? "PASS" : "FAIL", tests[index].name);
        if (!succeeded) {
            continue;
        }
        ++passed;
    }

    (void)printf("%u/%u tests passed\n", (unsigned int)passed,
                 (unsigned int)test_count);
    return passed == test_count ? 0 : 1;
}
