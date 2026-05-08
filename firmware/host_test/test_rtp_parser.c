#include "unity.h"
#include "rtp_parser.h"

#include <string.h>

static rtp_packet_t p;

void setUp(void)
{
    memset(&p, 0, sizeof p);
}

void tearDown(void) {}

static void test_minimal_v2_header(void)
{
    uint8_t pkt[12] = {
        0x80, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };
    TEST_ASSERT_TRUE(rtp_packet_parse(pkt, sizeof pkt, &p));
    TEST_ASSERT_EQUAL_UINT8(2, p.version);
    TEST_ASSERT_FALSE(p.padding);
    TEST_ASSERT_FALSE(p.extension);
    TEST_ASSERT_EQUAL_UINT8(0, p.cc);
    TEST_ASSERT_FALSE(p.marker);
    TEST_ASSERT_EQUAL_UINT8(0, p.payload_type);
    TEST_ASSERT_EQUAL_UINT(0, p.payload_len);
}

static void test_marker_and_payload_type(void)
{
    uint8_t pkt[12] = { 0x80, 0x80 | 26 };
    TEST_ASSERT_TRUE(rtp_packet_parse(pkt, sizeof pkt, &p));
    TEST_ASSERT_TRUE(p.marker);
    TEST_ASSERT_EQUAL_UINT8(26, p.payload_type);
}

static void test_seq_timestamp_ssrc_big_endian(void)
{
    uint8_t pkt[12] = {
        0x80, 0x00,
        0x12, 0x34,
        0xDE, 0xAD, 0xBE, 0xEF,
        0x01, 0x02, 0x03, 0x04,
    };
    TEST_ASSERT_TRUE(rtp_packet_parse(pkt, sizeof pkt, &p));
    TEST_ASSERT_EQUAL_UINT16(0x1234, p.sequence);
    TEST_ASSERT_EQUAL_UINT32(0xDEADBEEFu, p.timestamp);
    TEST_ASSERT_EQUAL_UINT32(0x01020304u, p.ssrc);
}

static void test_payload_extracted(void)
{
    uint8_t pkt[16] = {
        0x80, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0xAA, 0xBB, 0xCC, 0xDD,
    };
    TEST_ASSERT_TRUE(rtp_packet_parse(pkt, sizeof pkt, &p));
    TEST_ASSERT_EQUAL_UINT(4, p.payload_len);
    TEST_ASSERT_EQUAL_PTR(pkt + 12, p.payload);
    uint8_t expected[] = { 0xAA, 0xBB, 0xCC, 0xDD };
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, p.payload, 4);
}

static void test_csrc_offset_to_payload(void)
{
    uint8_t pkt[22] = {
        0x82, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x11, 0x11, 0x11, 0x11,
        0x22, 0x22, 0x22, 0x22,
        0xAA, 0xBB,
    };
    TEST_ASSERT_TRUE(rtp_packet_parse(pkt, sizeof pkt, &p));
    TEST_ASSERT_EQUAL_UINT8(2, p.cc);
    TEST_ASSERT_EQUAL_UINT(2, p.payload_len);
    TEST_ASSERT_EQUAL_PTR(pkt + 20, p.payload);
}

static void test_extension_offset_to_payload(void)
{
    uint8_t pkt[12 + 4 + 8 + 3] = {
        0x90, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00,
        0x00, 0x02,
        0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
        0xAA, 0xBB, 0xCC,
    };
    TEST_ASSERT_TRUE(rtp_packet_parse(pkt, sizeof pkt, &p));
    TEST_ASSERT_TRUE(p.extension);
    TEST_ASSERT_EQUAL_UINT(3, p.payload_len);
    TEST_ASSERT_EQUAL_PTR(pkt + 24, p.payload);
}

static void test_padding_stripped_from_payload_len(void)
{
    uint8_t pkt[17] = {
        0xA0, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0xAA, 0xBB,
        0xFF, 0xFF, 0x03,
    };
    TEST_ASSERT_TRUE(rtp_packet_parse(pkt, sizeof pkt, &p));
    TEST_ASSERT_TRUE(p.padding);
    TEST_ASSERT_EQUAL_UINT(2, p.payload_len);
    TEST_ASSERT_EQUAL_PTR(pkt + 12, p.payload);
}

static void test_reject_short_buffer(void)
{
    uint8_t pkt[8] = { 0x80 };
    TEST_ASSERT_FALSE(rtp_packet_parse(pkt, sizeof pkt, &p));
}

static void test_reject_wrong_version(void)
{
    uint8_t pkt[12] = { 0x40 };
    TEST_ASSERT_FALSE(rtp_packet_parse(pkt, sizeof pkt, &p));
}

static void test_reject_truncated_csrc(void)
{
    uint8_t pkt[16] = { 0x82, 0x00 };
    TEST_ASSERT_FALSE(rtp_packet_parse(pkt, sizeof pkt, &p));
}

static void test_reject_truncated_extension_header(void)
{
    uint8_t pkt[14] = { 0x90, 0x00 };
    TEST_ASSERT_FALSE(rtp_packet_parse(pkt, sizeof pkt, &p));
}

static void test_reject_extension_data_truncated(void)
{
    uint8_t pkt[16] = {
        0x90, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x0A,
    };
    TEST_ASSERT_FALSE(rtp_packet_parse(pkt, sizeof pkt, &p));
}

static void test_reject_padding_count_exceeds_payload(void)
{
    uint8_t pkt[14] = {
        0xA0, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0xAA, 0xFF,
    };
    TEST_ASSERT_FALSE(rtp_packet_parse(pkt, sizeof pkt, &p));
}

static void test_reject_padding_count_zero(void)
{
    uint8_t pkt[13] = {
        0xA0, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00,
    };
    TEST_ASSERT_FALSE(rtp_packet_parse(pkt, sizeof pkt, &p));
}

static void test_reject_padding_with_no_payload(void)
{
    uint8_t pkt[12] = { 0xA0, 0x00 };
    TEST_ASSERT_FALSE(rtp_packet_parse(pkt, sizeof pkt, &p));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_minimal_v2_header);
    RUN_TEST(test_marker_and_payload_type);
    RUN_TEST(test_seq_timestamp_ssrc_big_endian);
    RUN_TEST(test_payload_extracted);
    RUN_TEST(test_csrc_offset_to_payload);
    RUN_TEST(test_extension_offset_to_payload);
    RUN_TEST(test_padding_stripped_from_payload_len);
    RUN_TEST(test_reject_short_buffer);
    RUN_TEST(test_reject_wrong_version);
    RUN_TEST(test_reject_truncated_csrc);
    RUN_TEST(test_reject_truncated_extension_header);
    RUN_TEST(test_reject_extension_data_truncated);
    RUN_TEST(test_reject_padding_count_exceeds_payload);
    RUN_TEST(test_reject_padding_count_zero);
    RUN_TEST(test_reject_padding_with_no_payload);
    return UNITY_END();
}
