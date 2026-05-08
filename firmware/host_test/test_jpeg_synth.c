#include "unity.h"
#include "jpeg_synth.h"

#include <string.h>

static uint8_t out[8192];

void setUp(void)
{
    memset(out, 0, sizeof out);
}

void tearDown(void) {}

static const uint8_t *find_marker(const uint8_t *buf, size_t len, uint8_t marker)
{
    for (size_t i = 0; i + 1 < len; ++i) {
        if (buf[i] == 0xFF && buf[i + 1] == marker) {
            return buf + i;
        }
    }
    return NULL;
}

static size_t count_marker(const uint8_t *buf, size_t len, uint8_t marker)
{
    size_t n = 0;
    for (size_t i = 0; i + 1 < len; ++i) {
        if (buf[i] == 0xFF && buf[i + 1] == marker) {
            ++n;
            ++i;
        }
    }
    return n;
}

static void test_minimal_jpeg_with_inline_qt(void)
{
    uint8_t qt_luma[64];
    uint8_t qt_chroma[64];
    for (int i = 0; i < 64; ++i) qt_luma[i] = 16;
    for (int i = 0; i < 64; ++i) qt_chroma[i] = 17;
    uint8_t scan[] = { 0xAA, 0xBB, 0xCC };

    jpeg_synth_input_t in = {
        .width = 640, .height = 480,
        .type = 1, .q = 200,
        .qt_present = true,
        .qt_luma = qt_luma, .qt_chroma = qt_chroma,
        .scan = scan, .scan_len = sizeof scan,
    };

    int n = jpeg_synth_build(&in, out, sizeof out);
    TEST_ASSERT_GREATER_THAN(0, n);

    TEST_ASSERT_EQUAL_HEX8(0xFF, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0xD8, out[1]);
    TEST_ASSERT_EQUAL_HEX8(0xFF, out[n - 2]);
    TEST_ASSERT_EQUAL_HEX8(0xD9, out[n - 1]);
}

static void test_jpeg_contains_required_segments(void)
{
    uint8_t qt[64] = {0};
    uint8_t scan[] = { 0x00 };
    jpeg_synth_input_t in = {
        .width = 320, .height = 240,
        .type = 1, .q = 200,
        .qt_present = true, .qt_luma = qt, .qt_chroma = qt,
        .scan = scan, .scan_len = sizeof scan,
    };
    int n = jpeg_synth_build(&in, out, sizeof out);
    TEST_ASSERT_GREATER_THAN(0, n);

    TEST_ASSERT_EQUAL_UINT(2, count_marker(out, (size_t)n, 0xDB));
    TEST_ASSERT_EQUAL_UINT(4, count_marker(out, (size_t)n, 0xC4));
    TEST_ASSERT_EQUAL_UINT(1, count_marker(out, (size_t)n, 0xC0));
    TEST_ASSERT_EQUAL_UINT(1, count_marker(out, (size_t)n, 0xDA));
}

static void test_sof0_encodes_dimensions_big_endian(void)
{
    uint8_t qt[64] = {0};
    uint8_t scan[] = { 0x00 };
    jpeg_synth_input_t in = {
        .width = 1280, .height = 720,
        .type = 1, .q = 200,
        .qt_present = true, .qt_luma = qt, .qt_chroma = qt,
        .scan = scan, .scan_len = sizeof scan,
    };
    int n = jpeg_synth_build(&in, out, sizeof out);
    TEST_ASSERT_GREATER_THAN(0, n);

    const uint8_t *sof = find_marker(out, (size_t)n, 0xC0);
    TEST_ASSERT_NOT_NULL(sof);
    TEST_ASSERT_EQUAL_HEX8(0x02, sof[5]);
    TEST_ASSERT_EQUAL_HEX8(0xD0, sof[6]);
    TEST_ASSERT_EQUAL_HEX8(0x05, sof[7]);
    TEST_ASSERT_EQUAL_HEX8(0x00, sof[8]);
}

static void test_sof0_sampling_factors_for_type_1(void)
{
    uint8_t qt[64] = {0};
    uint8_t scan[] = { 0x00 };
    jpeg_synth_input_t in = {
        .width = 160, .height = 120,
        .type = 1, .q = 200,
        .qt_present = true, .qt_luma = qt, .qt_chroma = qt,
        .scan = scan, .scan_len = sizeof scan,
    };
    int n = jpeg_synth_build(&in, out, sizeof out);
    const uint8_t *sof = find_marker(out, (size_t)n, 0xC0);
    TEST_ASSERT_NOT_NULL(sof);
    TEST_ASSERT_EQUAL_HEX8(0x22, sof[11]);
    TEST_ASSERT_EQUAL_HEX8(0x11, sof[14]);
    TEST_ASSERT_EQUAL_HEX8(0x11, sof[17]);
}

static void test_sof0_sampling_factors_for_type_0(void)
{
    uint8_t qt[64] = {0};
    uint8_t scan[] = { 0x00 };
    jpeg_synth_input_t in = {
        .width = 160, .height = 120,
        .type = 0, .q = 200,
        .qt_present = true, .qt_luma = qt, .qt_chroma = qt,
        .scan = scan, .scan_len = sizeof scan,
    };
    int n = jpeg_synth_build(&in, out, sizeof out);
    const uint8_t *sof = find_marker(out, (size_t)n, 0xC0);
    TEST_ASSERT_NOT_NULL(sof);
    TEST_ASSERT_EQUAL_HEX8(0x21, sof[11]);
}

static void test_scan_data_appended_after_sos(void)
{
    uint8_t qt[64] = {0};
    uint8_t scan[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    jpeg_synth_input_t in = {
        .width = 160, .height = 120,
        .type = 1, .q = 200,
        .qt_present = true, .qt_luma = qt, .qt_chroma = qt,
        .scan = scan, .scan_len = sizeof scan,
    };
    int n = jpeg_synth_build(&in, out, sizeof out);
    TEST_ASSERT_GREATER_THAN(0, n);

    const uint8_t *sos = find_marker(out, (size_t)n, 0xDA);
    TEST_ASSERT_NOT_NULL(sos);
    const uint8_t *scan_start = sos + 2 + 12;
    TEST_ASSERT_EQUAL_HEX8_ARRAY(scan, scan_start, sizeof scan);
}

static void test_qt_below_128_uses_default_tables(void)
{
    uint8_t scan[] = { 0x00 };
    jpeg_synth_input_t in = {
        .width = 160, .height = 120,
        .type = 1, .q = 50,
        .qt_present = false,
        .scan = scan, .scan_len = sizeof scan,
    };
    int n = jpeg_synth_build(&in, out, sizeof out);
    TEST_ASSERT_GREATER_THAN(0, n);
    TEST_ASSERT_EQUAL_UINT(2, count_marker(out, (size_t)n, 0xDB));
}

static void test_buffer_too_small_returns_negative(void)
{
    uint8_t qt[64] = {0};
    uint8_t scan[] = { 0x00 };
    jpeg_synth_input_t in = {
        .width = 160, .height = 120,
        .type = 1, .q = 200,
        .qt_present = true, .qt_luma = qt, .qt_chroma = qt,
        .scan = scan, .scan_len = sizeof scan,
    };
    uint8_t tiny[64];
    TEST_ASSERT_LESS_THAN(0, jpeg_synth_build(&in, tiny, sizeof tiny));
}

static void test_default_quant_q50_returns_base_table(void)
{
    uint8_t luma[64];
    uint8_t chroma[64];
    jpeg_default_quant_tables(50, luma, chroma);
    TEST_ASSERT_EQUAL_UINT8(16, luma[0]);
    TEST_ASSERT_EQUAL_UINT8(11, luma[1]);
    TEST_ASSERT_EQUAL_UINT8(17, chroma[0]);
}

static void test_default_quant_q1_clamps_to_255(void)
{
    uint8_t luma[64];
    uint8_t chroma[64];
    jpeg_default_quant_tables(1, luma, chroma);
    for (int i = 0; i < 64; ++i) {
        TEST_ASSERT_EQUAL_UINT8(255, luma[i]);
    }
}

static void test_default_quant_q99_collapses_to_low_values(void)
{
    uint8_t luma[64];
    uint8_t chroma[64];
    jpeg_default_quant_tables(99, luma, chroma);
    for (int i = 0; i < 64; ++i) {
        TEST_ASSERT_TRUE_MESSAGE(luma[i] <= 3, "luma[i] expected <= 3 at Q=99");
    }
}

static void test_default_quant_q_zero_treated_as_q1(void)
{
    uint8_t a_luma[64], a_chroma[64];
    uint8_t b_luma[64], b_chroma[64];
    jpeg_default_quant_tables(0, a_luma, a_chroma);
    jpeg_default_quant_tables(1, b_luma, b_chroma);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(b_luma, a_luma, 64);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(b_chroma, a_chroma, 64);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_minimal_jpeg_with_inline_qt);
    RUN_TEST(test_jpeg_contains_required_segments);
    RUN_TEST(test_sof0_encodes_dimensions_big_endian);
    RUN_TEST(test_sof0_sampling_factors_for_type_1);
    RUN_TEST(test_sof0_sampling_factors_for_type_0);
    RUN_TEST(test_scan_data_appended_after_sos);
    RUN_TEST(test_qt_below_128_uses_default_tables);
    RUN_TEST(test_buffer_too_small_returns_negative);
    RUN_TEST(test_default_quant_q50_returns_base_table);
    RUN_TEST(test_default_quant_q1_clamps_to_255);
    RUN_TEST(test_default_quant_q99_collapses_to_low_values);
    RUN_TEST(test_default_quant_q_zero_treated_as_q1);
    return UNITY_END();
}
