#include "unity.h"
#include "rtsp_url.h"

#include <string.h>

static rtsp_url_t u;

void setUp(void)
{
    memset(&u, 0, sizeof u);
}

void tearDown(void) {}

static void test_basic_rtsp_url(void)
{
    TEST_ASSERT_TRUE(rtsp_url_parse("rtsp://192.168.1.50/stream1", &u));
    TEST_ASSERT_EQUAL_STRING("rtsp", u.scheme);
    TEST_ASSERT_EQUAL_STRING("192.168.1.50", u.host);
    TEST_ASSERT_EQUAL_UINT16(554, u.port);
    TEST_ASSERT_EQUAL_STRING("/stream1", u.path);
    TEST_ASSERT_EQUAL_STRING("", u.user);
    TEST_ASSERT_EQUAL_STRING("", u.password);
}

static void test_explicit_port(void)
{
    TEST_ASSERT_TRUE(rtsp_url_parse("rtsp://cam.local:8554/h264", &u));
    TEST_ASSERT_EQUAL_UINT16(8554, u.port);
    TEST_ASSERT_EQUAL_STRING("cam.local", u.host);
    TEST_ASSERT_EQUAL_STRING("/h264", u.path);
}

static void test_user_password(void)
{
    TEST_ASSERT_TRUE(rtsp_url_parse("rtsp://admin:secret@10.0.0.5:554/Streaming/Channels/101", &u));
    TEST_ASSERT_EQUAL_STRING("admin", u.user);
    TEST_ASSERT_EQUAL_STRING("secret", u.password);
    TEST_ASSERT_EQUAL_STRING("10.0.0.5", u.host);
    TEST_ASSERT_EQUAL_UINT16(554, u.port);
    TEST_ASSERT_EQUAL_STRING("/Streaming/Channels/101", u.path);
}

static void test_user_only(void)
{
    TEST_ASSERT_TRUE(rtsp_url_parse("rtsp://viewer@cam.lan/sub", &u));
    TEST_ASSERT_EQUAL_STRING("viewer", u.user);
    TEST_ASSERT_EQUAL_STRING("", u.password);
    TEST_ASSERT_EQUAL_STRING("cam.lan", u.host);
}

static void test_rtsps_scheme(void)
{
    TEST_ASSERT_TRUE(rtsp_url_parse("rtsps://cam.example.com/stream", &u));
    TEST_ASSERT_EQUAL_STRING("rtsps", u.scheme);
}

static void test_default_path_when_missing(void)
{
    TEST_ASSERT_TRUE(rtsp_url_parse("rtsp://cam.local", &u));
    TEST_ASSERT_EQUAL_STRING("/", u.path);
}

static void test_path_with_query_string(void)
{
    TEST_ASSERT_TRUE(rtsp_url_parse("rtsp://cam.local/stream?token=abc&fps=10", &u));
    TEST_ASSERT_EQUAL_STRING("/stream?token=abc&fps=10", u.path);
}

static void test_rejects_missing_scheme(void)
{
    TEST_ASSERT_FALSE(rtsp_url_parse("cam.local/stream", &u));
}

static void test_rejects_empty_host(void)
{
    TEST_ASSERT_FALSE(rtsp_url_parse("rtsp:///stream", &u));
}

static void test_rejects_invalid_port(void)
{
    TEST_ASSERT_FALSE(rtsp_url_parse("rtsp://cam.local:99999/s", &u));
    TEST_ASSERT_FALSE(rtsp_url_parse("rtsp://cam.local:abc/s", &u));
    TEST_ASSERT_FALSE(rtsp_url_parse("rtsp://cam.local:0/s", &u));
}

static void test_rejects_oversized_host(void)
{
    char buf[400];
    strcpy(buf, "rtsp://");
    size_t prefix = strlen(buf);
    for (size_t i = 0; i < 200; ++i) {
        buf[prefix + i] = 'a';
    }
    strcpy(buf + prefix + 200, "/s");
    TEST_ASSERT_FALSE(rtsp_url_parse(buf, &u));
}

static void test_password_with_special_in_path_separator(void)
{
    TEST_ASSERT_TRUE(rtsp_url_parse("rtsp://user:p%40ss@cam.local/x", &u));
    TEST_ASSERT_EQUAL_STRING("user", u.user);
    TEST_ASSERT_EQUAL_STRING("p%40ss", u.password);
    TEST_ASSERT_EQUAL_STRING("cam.local", u.host);
}

static void test_at_sign_in_path_does_not_confuse_userinfo(void)
{
    TEST_ASSERT_TRUE(rtsp_url_parse("rtsp://cam.local/stream@channel1", &u));
    TEST_ASSERT_EQUAL_STRING("", u.user);
    TEST_ASSERT_EQUAL_STRING("cam.local", u.host);
    TEST_ASSERT_EQUAL_STRING("/stream@channel1", u.path);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_basic_rtsp_url);
    RUN_TEST(test_explicit_port);
    RUN_TEST(test_user_password);
    RUN_TEST(test_user_only);
    RUN_TEST(test_rtsps_scheme);
    RUN_TEST(test_default_path_when_missing);
    RUN_TEST(test_path_with_query_string);
    RUN_TEST(test_rejects_missing_scheme);
    RUN_TEST(test_rejects_empty_host);
    RUN_TEST(test_rejects_invalid_port);
    RUN_TEST(test_rejects_oversized_host);
    RUN_TEST(test_password_with_special_in_path_separator);
    RUN_TEST(test_at_sign_in_path_does_not_confuse_userinfo);
    return UNITY_END();
}
