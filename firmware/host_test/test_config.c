#include "unity.h"
#include "config.h"

#include <string.h>

static cfg_handle_t *h;

void setUp(void)
{
    h = cfg_open();
}

void tearDown(void)
{
    cfg_close(h);
    h = NULL;
}

static void test_open_returns_non_null(void)
{
    TEST_ASSERT_NOT_NULL(h);
}

static void test_get_missing_str_returns_false(void)
{
    char buf[32];
    TEST_ASSERT_FALSE(cfg_get_str(h, "missing.k", buf, sizeof buf));
}

static void test_get_missing_u32_returns_false(void)
{
    uint32_t v = 0xDEADBEEFu;
    TEST_ASSERT_FALSE(cfg_get_u32(h, "missing.k", &v));
    TEST_ASSERT_EQUAL_UINT32(0xDEADBEEFu, v);
}

static void test_set_str_then_get_str_round_trip(void)
{
    TEST_ASSERT_TRUE(cfg_set_str(h, CFG_KEY_WIFI_SSID, "MyHomeWifi"));
    char buf[32] = {0};
    TEST_ASSERT_TRUE(cfg_get_str(h, CFG_KEY_WIFI_SSID, buf, sizeof buf));
    TEST_ASSERT_EQUAL_STRING("MyHomeWifi", buf);
}

static void test_set_u32_then_get_u32_round_trip(void)
{
    TEST_ASSERT_TRUE(cfg_set_u32(h, CFG_KEY_TONE_FREQ_HZ, 1200u));
    uint32_t out = 0;
    TEST_ASSERT_TRUE(cfg_get_u32(h, CFG_KEY_TONE_FREQ_HZ, &out));
    TEST_ASSERT_EQUAL_UINT32(1200u, out);
}

static void test_overwrite_str_replaces_value(void)
{
    cfg_set_str(h, CFG_KEY_CAM_ENT_URL, "rtsp://old");
    cfg_set_str(h, CFG_KEY_CAM_ENT_URL, "rtsp://new");
    char buf[32] = {0};
    cfg_get_str(h, CFG_KEY_CAM_ENT_URL, buf, sizeof buf);
    TEST_ASSERT_EQUAL_STRING("rtsp://new", buf);
}

static void test_overwrite_u32_replaces_value(void)
{
    cfg_set_u32(h, CFG_KEY_TONE_DEBOUNCE, 8u);
    cfg_set_u32(h, CFG_KEY_TONE_DEBOUNCE, 16u);
    uint32_t out = 0;
    cfg_get_u32(h, CFG_KEY_TONE_DEBOUNCE, &out);
    TEST_ASSERT_EQUAL_UINT32(16u, out);
}

static void test_get_str_with_undersized_buffer_fails(void)
{
    cfg_set_str(h, "key", "this is longer than buffer");
    char buf[8];
    TEST_ASSERT_FALSE(cfg_get_str(h, "key", buf, sizeof buf));
}

static void test_erase_removes_key(void)
{
    cfg_set_u32(h, CFG_KEY_TONE_FREQ_HZ, 1000);
    TEST_ASSERT_TRUE(cfg_erase(h, CFG_KEY_TONE_FREQ_HZ));
    uint32_t out;
    TEST_ASSERT_FALSE(cfg_get_u32(h, CFG_KEY_TONE_FREQ_HZ, &out));
}

static void test_erase_missing_key_returns_false(void)
{
    TEST_ASSERT_FALSE(cfg_erase(h, "no.such.key"));
}

static void test_multiple_keys_coexist(void)
{
    cfg_set_str(h, CFG_KEY_WIFI_SSID, "ssid-A");
    cfg_set_str(h, CFG_KEY_WIFI_PASS, "pass-B");
    cfg_set_u32(h, CFG_KEY_TONE_FREQ_HZ, 1100);

    char buf[32] = {0};
    cfg_get_str(h, CFG_KEY_WIFI_SSID, buf, sizeof buf);
    TEST_ASSERT_EQUAL_STRING("ssid-A", buf);

    memset(buf, 0, sizeof buf);
    cfg_get_str(h, CFG_KEY_WIFI_PASS, buf, sizeof buf);
    TEST_ASSERT_EQUAL_STRING("pass-B", buf);

    uint32_t freq = 0;
    cfg_get_u32(h, CFG_KEY_TONE_FREQ_HZ, &freq);
    TEST_ASSERT_EQUAL_UINT32(1100u, freq);
}

static void test_set_blob_then_get_blob_round_trip(void)
{
    const uint8_t payload[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02 };
    TEST_ASSERT_TRUE(cfg_set_blob(h, "blob.key", payload, sizeof payload));

    uint8_t out[16] = {0};
    size_t len = sizeof out;
    TEST_ASSERT_TRUE(cfg_get_blob(h, "blob.key", out, &len));
    TEST_ASSERT_EQUAL_UINT(sizeof payload, len);
    TEST_ASSERT_EQUAL_MEMORY(payload, out, sizeof payload);
}

static void test_get_blob_size_query_with_null_buffer(void)
{
    const uint8_t payload[] = { 1, 2, 3, 4, 5 };
    cfg_set_blob(h, "blob.key", payload, sizeof payload);

    size_t len = 0;
    TEST_ASSERT_TRUE(cfg_get_blob(h, "blob.key", NULL, &len));
    TEST_ASSERT_EQUAL_UINT(sizeof payload, len);
}

static void test_get_blob_with_undersized_buffer_reports_required_len(void)
{
    const uint8_t payload[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    cfg_set_blob(h, "blob.key", payload, sizeof payload);

    uint8_t small[3];
    size_t len = sizeof small;
    TEST_ASSERT_FALSE(cfg_get_blob(h, "blob.key", small, &len));
    TEST_ASSERT_EQUAL_UINT(sizeof payload, len);
}

static void test_commit_succeeds_on_empty_handle(void)
{
    TEST_ASSERT_TRUE(cfg_commit(h));
}

static void test_type_mismatch_returns_false(void)
{
    cfg_set_u32(h, "k", 42);
    char buf[8];
    TEST_ASSERT_FALSE(cfg_get_str(h, "k", buf, sizeof buf));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_open_returns_non_null);
    RUN_TEST(test_get_missing_str_returns_false);
    RUN_TEST(test_get_missing_u32_returns_false);
    RUN_TEST(test_set_str_then_get_str_round_trip);
    RUN_TEST(test_set_u32_then_get_u32_round_trip);
    RUN_TEST(test_overwrite_str_replaces_value);
    RUN_TEST(test_overwrite_u32_replaces_value);
    RUN_TEST(test_get_str_with_undersized_buffer_fails);
    RUN_TEST(test_erase_removes_key);
    RUN_TEST(test_erase_missing_key_returns_false);
    RUN_TEST(test_multiple_keys_coexist);
    RUN_TEST(test_set_blob_then_get_blob_round_trip);
    RUN_TEST(test_get_blob_size_query_with_null_buffer);
    RUN_TEST(test_get_blob_with_undersized_buffer_reports_required_len);
    RUN_TEST(test_commit_succeeds_on_empty_handle);
    RUN_TEST(test_type_mismatch_returns_false);
    return UNITY_END();
}
