#include "unity.h"
#include "rtsp_request.h"

#include <string.h>

static char buf[1024];

void setUp(void)
{
    memset(buf, 0, sizeof buf);
}

void tearDown(void) {}

static void assert_contains(const char *haystack, const char *needle)
{
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(haystack, needle), needle);
}

static void test_options_request_minimal(void)
{
    rtsp_request_t r = {
        .method = RTSP_M_OPTIONS,
        .uri = "rtsp://cam.local/stream",
        .cseq = 1,
    };
    int n = rtsp_request_build(&r, buf, sizeof buf);
    TEST_ASSERT_GREATER_THAN(0, n);
    assert_contains(buf, "OPTIONS rtsp://cam.local/stream RTSP/1.0\r\n");
    assert_contains(buf, "CSeq: 1\r\n");
    TEST_ASSERT_EQUAL_STRING("\r\n", buf + n - 2);
}

static void test_describe_request_with_user_agent_and_accept(void)
{
    rtsp_request_t r = {
        .method = RTSP_M_DESCRIBE,
        .uri = "rtsp://cam.local/stream",
        .cseq = 2,
        .user_agent = "smart-intercom/0.1",
        .accept = "application/sdp",
    };
    int n = rtsp_request_build(&r, buf, sizeof buf);
    TEST_ASSERT_GREATER_THAN(0, n);
    assert_contains(buf, "DESCRIBE rtsp://cam.local/stream RTSP/1.0\r\n");
    assert_contains(buf, "CSeq: 2\r\n");
    assert_contains(buf, "User-Agent: smart-intercom/0.1\r\n");
    assert_contains(buf, "Accept: application/sdp\r\n");
}

static void test_setup_request_with_transport(void)
{
    rtsp_request_t r = {
        .method = RTSP_M_SETUP,
        .uri = "rtsp://cam.local/stream/trackID=0",
        .cseq = 3,
        .transport = "RTP/AVP/TCP;unicast;interleaved=0-1",
    };
    int n = rtsp_request_build(&r, buf, sizeof buf);
    TEST_ASSERT_GREATER_THAN(0, n);
    assert_contains(buf, "SETUP rtsp://cam.local/stream/trackID=0 RTSP/1.0\r\n");
    assert_contains(buf, "Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n");
}

static void test_play_request_includes_range_and_session(void)
{
    rtsp_request_t r = {
        .method = RTSP_M_PLAY,
        .uri = "rtsp://cam.local/stream",
        .cseq = 4,
        .session = "ABCD1234",
    };
    int n = rtsp_request_build(&r, buf, sizeof buf);
    TEST_ASSERT_GREATER_THAN(0, n);
    assert_contains(buf, "PLAY rtsp://cam.local/stream RTSP/1.0\r\n");
    assert_contains(buf, "Session: ABCD1234\r\n");
    assert_contains(buf, "Range: npt=0.000-\r\n");
}

static void test_teardown_request(void)
{
    rtsp_request_t r = {
        .method = RTSP_M_TEARDOWN,
        .uri = "rtsp://cam.local/stream",
        .cseq = 5,
        .session = "ABCD1234",
    };
    int n = rtsp_request_build(&r, buf, sizeof buf);
    TEST_ASSERT_GREATER_THAN(0, n);
    assert_contains(buf, "TEARDOWN rtsp://cam.local/stream RTSP/1.0\r\n");
    assert_contains(buf, "Session: ABCD1234\r\n");
}

static void test_authorization_header_emitted(void)
{
    rtsp_request_t r = {
        .method = RTSP_M_DESCRIBE,
        .uri = "rtsp://cam.local/stream",
        .cseq = 6,
        .authorization = "Digest username=\"a\", realm=\"x\", nonce=\"y\", uri=\"/s\", response=\"abc\"",
    };
    int n = rtsp_request_build(&r, buf, sizeof buf);
    TEST_ASSERT_GREATER_THAN(0, n);
    assert_contains(buf, "Authorization: Digest username=\"a\"");
}

static void test_buffer_too_small_returns_negative(void)
{
    rtsp_request_t r = {
        .method = RTSP_M_OPTIONS,
        .uri = "rtsp://cam.local/stream",
        .cseq = 1,
    };
    char tiny[16];
    TEST_ASSERT_LESS_THAN(0, rtsp_request_build(&r, tiny, sizeof tiny));
}

static void test_method_name_lookup(void)
{
    TEST_ASSERT_EQUAL_STRING("OPTIONS", rtsp_method_name(RTSP_M_OPTIONS));
    TEST_ASSERT_EQUAL_STRING("DESCRIBE", rtsp_method_name(RTSP_M_DESCRIBE));
    TEST_ASSERT_EQUAL_STRING("SETUP", rtsp_method_name(RTSP_M_SETUP));
    TEST_ASSERT_EQUAL_STRING("PLAY", rtsp_method_name(RTSP_M_PLAY));
    TEST_ASSERT_EQUAL_STRING("PAUSE", rtsp_method_name(RTSP_M_PAUSE));
    TEST_ASSERT_EQUAL_STRING("TEARDOWN", rtsp_method_name(RTSP_M_TEARDOWN));
    TEST_ASSERT_EQUAL_STRING("GET_PARAMETER", rtsp_method_name(RTSP_M_GET_PARAMETER));
}

static void test_request_terminates_with_blank_line(void)
{
    rtsp_request_t r = {
        .method = RTSP_M_OPTIONS,
        .uri = "rtsp://cam.local/",
        .cseq = 1,
    };
    int n = rtsp_request_build(&r, buf, sizeof buf);
    TEST_ASSERT_GREATER_THAN(4, n);
    TEST_ASSERT_EQUAL_MEMORY("\r\n\r\n", buf + n - 4, 4);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_options_request_minimal);
    RUN_TEST(test_describe_request_with_user_agent_and_accept);
    RUN_TEST(test_setup_request_with_transport);
    RUN_TEST(test_play_request_includes_range_and_session);
    RUN_TEST(test_teardown_request);
    RUN_TEST(test_authorization_header_emitted);
    RUN_TEST(test_buffer_too_small_returns_negative);
    RUN_TEST(test_method_name_lookup);
    RUN_TEST(test_request_terminates_with_blank_line);
    return UNITY_END();
}
