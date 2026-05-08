#include "unity.h"
#include "rtsp_response.h"

#include <stdio.h>
#include <string.h>

static rtsp_response_t out;

void setUp(void)
{
    memset(&out, 0, sizeof out);
}

void tearDown(void) {}

static void assert_contains(const char *haystack, const char *needle)
{
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(haystack, needle), needle);
}

static void test_minimal_200_response(void)
{
    const char *r = "RTSP/1.0 200 OK\r\nCSeq: 1\r\n\r\n";
    TEST_ASSERT_TRUE(rtsp_response_parse(r, strlen(r), &out));
    TEST_ASSERT_EQUAL(200, out.status_code);
    TEST_ASSERT_EQUAL_STRING("OK", out.reason);
    TEST_ASSERT_EQUAL_UINT32(1, out.cseq);
    TEST_ASSERT_NULL(out.body);
    TEST_ASSERT_EQUAL_UINT(0, out.body_len);
}

static void test_401_unauthorized_with_digest_challenge(void)
{
    const char *r = "RTSP/1.0 401 Unauthorized\r\n"
                    "CSeq: 1\r\n"
                    "WWW-Authenticate: Digest realm=\"x\", nonce=\"y\"\r\n\r\n";
    TEST_ASSERT_TRUE(rtsp_response_parse(r, strlen(r), &out));
    TEST_ASSERT_EQUAL(401, out.status_code);
    TEST_ASSERT_EQUAL_STRING("Unauthorized", out.reason);
    assert_contains(out.www_authenticate, "Digest");
    assert_contains(out.www_authenticate, "nonce=\"y\"");
}

static void test_session_with_timeout(void)
{
    const char *r = "RTSP/1.0 200 OK\r\n"
                    "CSeq: 3\r\n"
                    "Session: ABCD1234; timeout=60\r\n\r\n";
    TEST_ASSERT_TRUE(rtsp_response_parse(r, strlen(r), &out));
    TEST_ASSERT_EQUAL_STRING("ABCD1234", out.session);
    TEST_ASSERT_EQUAL_UINT32(60, out.session_timeout);
}

static void test_session_without_timeout(void)
{
    const char *r = "RTSP/1.0 200 OK\r\n"
                    "CSeq: 1\r\n"
                    "Session: BIGSESSION\r\n\r\n";
    TEST_ASSERT_TRUE(rtsp_response_parse(r, strlen(r), &out));
    TEST_ASSERT_EQUAL_STRING("BIGSESSION", out.session);
    TEST_ASSERT_EQUAL_UINT32(0, out.session_timeout);
}

static void test_setup_response_with_transport(void)
{
    const char *r = "RTSP/1.0 200 OK\r\n"
                    "CSeq: 3\r\n"
                    "Session: XYZ\r\n"
                    "Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n";
    TEST_ASSERT_TRUE(rtsp_response_parse(r, strlen(r), &out));
    TEST_ASSERT_EQUAL_STRING("RTP/AVP/TCP;unicast;interleaved=0-1", out.transport);
}

static void test_describe_response_with_sdp_body(void)
{
    const char body[] = "v=0\r\no=- 12345 1 IN IP4 192.168.1.50\r\n";
    char buf[1024];
    int n = snprintf(buf, sizeof buf,
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 2\r\n"
        "Content-Type: application/sdp\r\n"
        "Content-Length: %u\r\n\r\n%s",
        (unsigned)(sizeof body - 1), body);
    TEST_ASSERT_TRUE(rtsp_response_parse(buf, (size_t)n, &out));
    TEST_ASSERT_EQUAL_STRING("application/sdp", out.content_type);
    TEST_ASSERT_EQUAL_UINT(sizeof body - 1, out.body_len);
    TEST_ASSERT_NOT_NULL(out.body);
    TEST_ASSERT_EQUAL_MEMORY(body, out.body, sizeof body - 1);
}

static void test_case_insensitive_headers(void)
{
    const char *r = "RTSP/1.0 200 OK\r\n"
                    "CSEQ: 7\r\n"
                    "session: id1\r\n"
                    "TRANSPORT: foo\r\n\r\n";
    TEST_ASSERT_TRUE(rtsp_response_parse(r, strlen(r), &out));
    TEST_ASSERT_EQUAL_UINT32(7, out.cseq);
    TEST_ASSERT_EQUAL_STRING("id1", out.session);
    TEST_ASSERT_EQUAL_STRING("foo", out.transport);
}

static void test_unknown_headers_are_ignored(void)
{
    const char *r = "RTSP/1.0 200 OK\r\n"
                    "CSeq: 1\r\n"
                    "Server: BlahCam/1.0\r\n"
                    "Date: Fri, 7 May 2026 15:00:00 GMT\r\n\r\n";
    TEST_ASSERT_TRUE(rtsp_response_parse(r, strlen(r), &out));
    TEST_ASSERT_EQUAL_UINT32(1, out.cseq);
}

static void test_incomplete_response_rejected(void)
{
    const char *r = "RTSP/1.0 200 OK\r\nCSeq: 1\r\n";
    TEST_ASSERT_FALSE(rtsp_response_parse(r, strlen(r), &out));
}

static void test_malformed_status_line_rejected(void)
{
    const char *r = "HTTP/1.1 200 OK\r\n\r\n";
    TEST_ASSERT_FALSE(rtsp_response_parse(r, strlen(r), &out));
}

static void test_non_numeric_status_rejected(void)
{
    const char *r = "RTSP/1.0 abc OK\r\n\r\n";
    TEST_ASSERT_FALSE(rtsp_response_parse(r, strlen(r), &out));
}

static void test_body_shorter_than_content_length_rejected(void)
{
    const char *r = "RTSP/1.0 200 OK\r\nCSeq: 1\r\nContent-Length: 100\r\n\r\nshort";
    TEST_ASSERT_FALSE(rtsp_response_parse(r, strlen(r), &out));
}

static void test_options_response_public_header_ignored_safely(void)
{
    const char *r = "RTSP/1.0 200 OK\r\n"
                    "CSeq: 1\r\n"
                    "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n\r\n";
    TEST_ASSERT_TRUE(rtsp_response_parse(r, strlen(r), &out));
    TEST_ASSERT_EQUAL(200, out.status_code);
}

static void test_value_with_leading_whitespace(void)
{
    const char *r = "RTSP/1.0 200 OK\r\nCSeq:   42  \r\n\r\n";
    TEST_ASSERT_TRUE(rtsp_response_parse(r, strlen(r), &out));
    TEST_ASSERT_EQUAL_UINT32(42, out.cseq);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_minimal_200_response);
    RUN_TEST(test_401_unauthorized_with_digest_challenge);
    RUN_TEST(test_session_with_timeout);
    RUN_TEST(test_session_without_timeout);
    RUN_TEST(test_setup_response_with_transport);
    RUN_TEST(test_describe_response_with_sdp_body);
    RUN_TEST(test_case_insensitive_headers);
    RUN_TEST(test_unknown_headers_are_ignored);
    RUN_TEST(test_incomplete_response_rejected);
    RUN_TEST(test_malformed_status_line_rejected);
    RUN_TEST(test_non_numeric_status_rejected);
    RUN_TEST(test_body_shorter_than_content_length_rejected);
    RUN_TEST(test_options_response_public_header_ignored_safely);
    RUN_TEST(test_value_with_leading_whitespace);
    return UNITY_END();
}
