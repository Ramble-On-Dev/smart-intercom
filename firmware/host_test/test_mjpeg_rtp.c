#include "unity.h"
#include "mjpeg_rtp.h"

#include <string.h>

static mjpeg_rtp_assembler_t *a;

void setUp(void)
{
    a = mjpeg_rtp_create(64 * 1024);
}

void tearDown(void)
{
    mjpeg_rtp_destroy(a);
    a = NULL;
}

#define MAX_PAYLOAD 8192

static size_t build_payload(uint8_t *buf,
                            uint32_t fragment_offset,
                            uint8_t type, uint8_t q,
                            uint8_t w8, uint8_t h8,
                            bool include_qt,
                            const uint8_t *scan, size_t scan_len)
{
    size_t pos = 0;
    buf[pos++] = 0;
    buf[pos++] = (uint8_t)((fragment_offset >> 16) & 0xFF);
    buf[pos++] = (uint8_t)((fragment_offset >> 8) & 0xFF);
    buf[pos++] = (uint8_t)(fragment_offset & 0xFF);
    buf[pos++] = type;
    buf[pos++] = q;
    buf[pos++] = w8;
    buf[pos++] = h8;

    if (include_qt && fragment_offset == 0 && q >= 128) {
        buf[pos++] = 0;
        buf[pos++] = 0;
        buf[pos++] = 0;
        buf[pos++] = 128;
        for (int i = 0; i < 128; ++i) {
            buf[pos++] = (uint8_t)(i + 1);
        }
    }

    memcpy(buf + pos, scan, scan_len);
    pos += scan_len;
    return pos;
}

static void test_single_fragment_frame_with_inline_qt(void)
{
    uint8_t scan[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE };
    uint8_t pkt[MAX_PAYLOAD];
    size_t plen = build_payload(pkt, 0, 1, 200, 80, 60, true, scan, sizeof scan);

    mjpeg_rtp_status_t s = mjpeg_rtp_feed(a, pkt, plen, true);
    TEST_ASSERT_EQUAL(MJPEG_RTP_FRAME_READY, s);
    TEST_ASSERT_EQUAL_UINT16(80 * 8, mjpeg_rtp_width(a));
    TEST_ASSERT_EQUAL_UINT16(60 * 8, mjpeg_rtp_height(a));
    TEST_ASSERT_EQUAL_UINT8(1, mjpeg_rtp_type(a));
    TEST_ASSERT_EQUAL_UINT8(200, mjpeg_rtp_q(a));
    TEST_ASSERT_TRUE(mjpeg_rtp_qt_present(a));
    TEST_ASSERT_EQUAL_UINT(sizeof scan, mjpeg_rtp_scan_size(a));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(scan, mjpeg_rtp_scan_data(a), sizeof scan);

    uint8_t expected_luma[64];
    uint8_t expected_chroma[64];
    for (int i = 0; i < 64; ++i) expected_luma[i] = (uint8_t)(i + 1);
    for (int i = 0; i < 64; ++i) expected_chroma[i] = (uint8_t)(i + 65);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_luma, mjpeg_rtp_qt_luma(a), 64);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_chroma, mjpeg_rtp_qt_chroma(a), 64);
}

static void test_multi_fragment_frame_assembled(void)
{
    uint8_t scan_a[] = { 0x10, 0x20, 0x30, 0x40 };
    uint8_t scan_b[] = { 0x50, 0x60 };
    uint8_t scan_c[] = { 0x70, 0x80, 0x90 };
    uint8_t pkt[MAX_PAYLOAD];

    size_t plen = build_payload(pkt, 0, 1, 200, 80, 60, true, scan_a, sizeof scan_a);
    TEST_ASSERT_EQUAL(MJPEG_RTP_OK, mjpeg_rtp_feed(a, pkt, plen, false));

    plen = build_payload(pkt, sizeof scan_a, 1, 200, 80, 60, false, scan_b, sizeof scan_b);
    TEST_ASSERT_EQUAL(MJPEG_RTP_OK, mjpeg_rtp_feed(a, pkt, plen, false));

    plen = build_payload(pkt, sizeof scan_a + sizeof scan_b, 1, 200, 80, 60, false, scan_c, sizeof scan_c);
    TEST_ASSERT_EQUAL(MJPEG_RTP_FRAME_READY, mjpeg_rtp_feed(a, pkt, plen, true));

    uint8_t expected[9] = { 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90 };
    TEST_ASSERT_EQUAL_UINT(9, mjpeg_rtp_scan_size(a));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, mjpeg_rtp_scan_data(a), 9);
}

static void test_out_of_order_fragment_dropped(void)
{
    uint8_t scan_a[] = { 0xAA, 0xBB };
    uint8_t pkt[MAX_PAYLOAD];

    size_t plen = build_payload(pkt, 0, 1, 200, 80, 60, true, scan_a, sizeof scan_a);
    TEST_ASSERT_EQUAL(MJPEG_RTP_OK, mjpeg_rtp_feed(a, pkt, plen, false));

    uint8_t scan_b[] = { 0xCC };
    plen = build_payload(pkt, 999, 1, 200, 80, 60, false, scan_b, sizeof scan_b);
    TEST_ASSERT_EQUAL(MJPEG_RTP_DROPPED, mjpeg_rtp_feed(a, pkt, plen, false));
}

static void test_first_fragment_must_have_offset_zero(void)
{
    uint8_t scan[] = { 0xAA };
    uint8_t pkt[MAX_PAYLOAD];
    size_t plen = build_payload(pkt, 100, 1, 200, 80, 60, false, scan, sizeof scan);
    TEST_ASSERT_EQUAL(MJPEG_RTP_DROPPED, mjpeg_rtp_feed(a, pkt, plen, false));
}

static void test_short_payload_rejected(void)
{
    uint8_t pkt[6] = {0};
    TEST_ASSERT_EQUAL(MJPEG_RTP_DROPPED, mjpeg_rtp_feed(a, pkt, sizeof pkt, true));
}

static void test_restart_marker_type_rejected(void)
{
    uint8_t scan[] = { 0xAA };
    uint8_t pkt[MAX_PAYLOAD];
    size_t plen = build_payload(pkt, 0, 64, 200, 80, 60, true, scan, sizeof scan);
    TEST_ASSERT_EQUAL(MJPEG_RTP_DROPPED, mjpeg_rtp_feed(a, pkt, plen, true));
}

static void test_q_below_128_no_inline_qt(void)
{
    uint8_t scan[] = { 0xAA, 0xBB };
    uint8_t pkt[MAX_PAYLOAD];
    size_t plen = build_payload(pkt, 0, 1, 50, 80, 60, false, scan, sizeof scan);
    TEST_ASSERT_EQUAL(MJPEG_RTP_FRAME_READY, mjpeg_rtp_feed(a, pkt, plen, true));
    TEST_ASSERT_EQUAL_UINT8(50, mjpeg_rtp_q(a));
    TEST_ASSERT_FALSE(mjpeg_rtp_qt_present(a));
    TEST_ASSERT_EQUAL_UINT(sizeof scan, mjpeg_rtp_scan_size(a));
}

static void test_buffer_overflow_drops(void)
{
    mjpeg_rtp_destroy(a);
    a = mjpeg_rtp_create(32);

    uint8_t big_scan[64] = {0};
    uint8_t pkt[MAX_PAYLOAD];
    size_t plen = build_payload(pkt, 0, 1, 200, 80, 60, true, big_scan, sizeof big_scan);
    TEST_ASSERT_EQUAL(MJPEG_RTP_DROPPED, mjpeg_rtp_feed(a, pkt, plen, true));
}

static void test_reset_starts_fresh(void)
{
    uint8_t scan[] = { 0xAA, 0xBB };
    uint8_t pkt[MAX_PAYLOAD];
    size_t plen = build_payload(pkt, 0, 1, 200, 80, 60, true, scan, sizeof scan);
    mjpeg_rtp_feed(a, pkt, plen, true);
    mjpeg_rtp_reset(a);
    TEST_ASSERT_EQUAL_UINT(0, mjpeg_rtp_scan_size(a));
    TEST_ASSERT_FALSE(mjpeg_rtp_qt_present(a));
}

static void test_dimensions_computed_in_pixels(void)
{
    uint8_t scan[] = { 0x00 };
    uint8_t pkt[MAX_PAYLOAD];
    size_t plen = build_payload(pkt, 0, 1, 200, 160, 90, true, scan, sizeof scan);
    mjpeg_rtp_feed(a, pkt, plen, true);
    TEST_ASSERT_EQUAL_UINT16(1280, mjpeg_rtp_width(a));
    TEST_ASSERT_EQUAL_UINT16(720, mjpeg_rtp_height(a));
}

static void test_new_frame_after_completed(void)
{
    uint8_t scan_a[] = { 0xAA };
    uint8_t scan_b[] = { 0xBB, 0xCC };
    uint8_t pkt[MAX_PAYLOAD];

    size_t plen = build_payload(pkt, 0, 1, 200, 80, 60, true, scan_a, sizeof scan_a);
    TEST_ASSERT_EQUAL(MJPEG_RTP_FRAME_READY, mjpeg_rtp_feed(a, pkt, plen, true));

    plen = build_payload(pkt, 0, 1, 200, 80, 60, true, scan_b, sizeof scan_b);
    TEST_ASSERT_EQUAL(MJPEG_RTP_FRAME_READY, mjpeg_rtp_feed(a, pkt, plen, true));
    TEST_ASSERT_EQUAL_UINT(sizeof scan_b, mjpeg_rtp_scan_size(a));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(scan_b, mjpeg_rtp_scan_data(a), sizeof scan_b);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_single_fragment_frame_with_inline_qt);
    RUN_TEST(test_multi_fragment_frame_assembled);
    RUN_TEST(test_out_of_order_fragment_dropped);
    RUN_TEST(test_first_fragment_must_have_offset_zero);
    RUN_TEST(test_short_payload_rejected);
    RUN_TEST(test_restart_marker_type_rejected);
    RUN_TEST(test_q_below_128_no_inline_qt);
    RUN_TEST(test_buffer_overflow_drops);
    RUN_TEST(test_reset_starts_fresh);
    RUN_TEST(test_dimensions_computed_in_pixels);
    RUN_TEST(test_new_frame_after_completed);
    return UNITY_END();
}
