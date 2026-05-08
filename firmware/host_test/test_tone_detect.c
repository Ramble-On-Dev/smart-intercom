#include "unity.h"
#include "tone_detect.h"

#include <math.h>
#include <stdint.h>

#define BS              200
#define FS              8000.0f
#define F_TARGET        1000.0f
#define F_OFF           1500.0f
#define AMP             10000.0f
#define PI_F            3.14159265358979323846f

static int16_t g_buf[BS];

static void fill_sine(int16_t *buf, size_t n, float freq, float fs, float amp)
{
    for (size_t i = 0; i < n; ++i) {
        float t = (float)i / fs;
        buf[i] = (int16_t)(amp * sinf(2.0f * PI_F * freq * t));
    }
}

static void fill_silence(int16_t *buf, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        buf[i] = 0;
    }
}

void setUp(void) {}
void tearDown(void) {}

static void test_silence_yields_near_zero_magnitude(void)
{
    fill_silence(g_buf, BS);
    float mag2 = goertzel_magnitude_squared(g_buf, BS, F_TARGET, FS);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 0.0f, mag2);
}

static void test_target_frequency_yields_large_magnitude(void)
{
    fill_sine(g_buf, BS, F_TARGET, FS, AMP);
    float mag2 = goertzel_magnitude_squared(g_buf, BS, F_TARGET, FS);
    TEST_ASSERT_GREATER_THAN_FLOAT(1e10f, mag2);
}

static void test_off_frequency_signal_yields_smaller_magnitude(void)
{
    int16_t target_buf[BS];
    int16_t off_buf[BS];
    fill_sine(target_buf, BS, F_TARGET, FS, AMP);
    fill_sine(off_buf, BS, F_OFF, FS, AMP);
    float on_target  = goertzel_magnitude_squared(target_buf, BS, F_TARGET, FS);
    float off_target = goertzel_magnitude_squared(off_buf,    BS, F_TARGET, FS);
    TEST_ASSERT_TRUE(on_target > 100.0f * off_target);
}

static void test_detector_below_threshold_does_not_fire(void)
{
    tone_detector_t d;
    td_init(&d, F_TARGET, FS, BS, 1e12f, 8);
    fill_sine(g_buf, BS, F_TARGET, FS, 100.0f);
    for (int i = 0; i < 20; ++i) {
        TEST_ASSERT_FALSE(td_process(&d, g_buf));
    }
}

static void test_detector_fires_on_nth_above_threshold(void)
{
    tone_detector_t d;
    td_init(&d, F_TARGET, FS, BS, 1e8f, 8);
    fill_sine(g_buf, BS, F_TARGET, FS, AMP);
    for (int i = 0; i < 7; ++i) {
        TEST_ASSERT_FALSE(td_process(&d, g_buf));
    }
    TEST_ASSERT_TRUE(td_process(&d, g_buf));
}

static void test_detector_resets_consecutive_on_below_threshold_frame(void)
{
    tone_detector_t d;
    td_init(&d, F_TARGET, FS, BS, 1e8f, 8);

    int16_t loud[BS];
    int16_t quiet[BS];
    fill_sine(loud, BS, F_TARGET, FS, AMP);
    fill_silence(quiet, BS);

    for (int i = 0; i < 5; ++i) {
        TEST_ASSERT_FALSE(td_process(&d, loud));
    }
    TEST_ASSERT_FALSE(td_process(&d, quiet));
    for (int i = 0; i < 7; ++i) {
        TEST_ASSERT_FALSE(td_process(&d, loud));
    }
    TEST_ASSERT_TRUE(td_process(&d, loud));
}

static void test_detector_self_resets_after_fire(void)
{
    tone_detector_t d;
    td_init(&d, F_TARGET, FS, BS, 1e8f, 4);
    fill_sine(g_buf, BS, F_TARGET, FS, AMP);

    for (int i = 0; i < 3; ++i) {
        TEST_ASSERT_FALSE(td_process(&d, g_buf));
    }
    TEST_ASSERT_TRUE(td_process(&d, g_buf));

    for (int i = 0; i < 3; ++i) {
        TEST_ASSERT_FALSE(td_process(&d, g_buf));
    }
    TEST_ASSERT_TRUE(td_process(&d, g_buf));
}

static void test_reset_clears_consecutive(void)
{
    tone_detector_t d;
    td_init(&d, F_TARGET, FS, BS, 1e8f, 4);
    fill_sine(g_buf, BS, F_TARGET, FS, AMP);

    td_process(&d, g_buf);
    td_process(&d, g_buf);
    td_reset(&d);

    for (int i = 0; i < 3; ++i) {
        TEST_ASSERT_FALSE(td_process(&d, g_buf));
    }
    TEST_ASSERT_TRUE(td_process(&d, g_buf));
}

static void test_init_zeroes_consecutive(void)
{
    tone_detector_t d;
    d.consecutive = 99;
    td_init(&d, F_TARGET, FS, BS, 1e8f, 4);
    TEST_ASSERT_EQUAL_UINT(0, d.consecutive);
}

static void test_off_frequency_signal_does_not_fire_at_target(void)
{
    tone_detector_t d;
    td_init(&d, F_TARGET, FS, BS, 1e10f, 4);
    fill_sine(g_buf, BS, F_OFF, FS, AMP);
    for (int i = 0; i < 20; ++i) {
        TEST_ASSERT_FALSE(td_process(&d, g_buf));
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_silence_yields_near_zero_magnitude);
    RUN_TEST(test_target_frequency_yields_large_magnitude);
    RUN_TEST(test_off_frequency_signal_yields_smaller_magnitude);
    RUN_TEST(test_detector_below_threshold_does_not_fire);
    RUN_TEST(test_detector_fires_on_nth_above_threshold);
    RUN_TEST(test_detector_resets_consecutive_on_below_threshold_frame);
    RUN_TEST(test_detector_self_resets_after_fire);
    RUN_TEST(test_reset_clears_consecutive);
    RUN_TEST(test_init_zeroes_consecutive);
    RUN_TEST(test_off_frequency_signal_does_not_fire_at_target);
    return UNITY_END();
}
