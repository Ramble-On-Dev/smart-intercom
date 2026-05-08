#include "config.h"

#include "nvs.h"
#include "nvs_flash.h"

#include <stdlib.h>

static const char *NS = "intercom";
static bool s_nvs_init = false;

struct cfg_handle {
    nvs_handle_t handle;
};

static esp_err_t ensure_nvs_init(void)
{
    if (s_nvs_init) {
        return ESP_OK;
    }
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err == ESP_OK) {
        s_nvs_init = true;
    }
    return err;
}

cfg_handle_t *cfg_open(void)
{
    if (ensure_nvs_init() != ESP_OK) {
        return NULL;
    }
    cfg_handle_t *h = calloc(1, sizeof *h);
    if (!h) {
        return NULL;
    }
    if (nvs_open(NS, NVS_READWRITE, &h->handle) != ESP_OK) {
        free(h);
        return NULL;
    }
    return h;
}

void cfg_close(cfg_handle_t *h)
{
    if (!h) {
        return;
    }
    nvs_close(h->handle);
    free(h);
}

bool cfg_get_str(cfg_handle_t *h, const char *key, char *buf, size_t buflen)
{
    size_t len = buflen;
    return nvs_get_str(h->handle, key, buf, &len) == ESP_OK;
}

bool cfg_set_str(cfg_handle_t *h, const char *key, const char *val)
{
    return nvs_set_str(h->handle, key, val) == ESP_OK;
}

bool cfg_get_u32(cfg_handle_t *h, const char *key, uint32_t *out)
{
    return nvs_get_u32(h->handle, key, out) == ESP_OK;
}

bool cfg_set_u32(cfg_handle_t *h, const char *key, uint32_t val)
{
    return nvs_set_u32(h->handle, key, val) == ESP_OK;
}

bool cfg_get_blob(cfg_handle_t *h, const char *key, void *buf, size_t *inout_len)
{
    return nvs_get_blob(h->handle, key, buf, inout_len) == ESP_OK;
}

bool cfg_set_blob(cfg_handle_t *h, const char *key, const void *buf, size_t len)
{
    return nvs_set_blob(h->handle, key, buf, len) == ESP_OK;
}

bool cfg_erase(cfg_handle_t *h, const char *key)
{
    return nvs_erase_key(h->handle, key) == ESP_OK;
}

bool cfg_commit(cfg_handle_t *h)
{
    return nvs_commit(h->handle) == ESP_OK;
}
