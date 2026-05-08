#ifndef SMART_INTERCOM_CONFIG_H
#define SMART_INTERCOM_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_KEY_WIFI_SSID      "wifi.ssid"
#define CFG_KEY_WIFI_PASS      "wifi.pass"
#define CFG_KEY_CAM_ENT_URL    "cam.ent.url"
#define CFG_KEY_CAM_DOOR_URL   "cam.door.url"
#define CFG_KEY_TONE_FREQ_HZ   "tone.freq"
#define CFG_KEY_TONE_THRESHOLD "tone.thresh"
#define CFG_KEY_TONE_DEBOUNCE  "tone.deb"

typedef struct cfg_handle cfg_handle_t;

cfg_handle_t *cfg_open(void);
void cfg_close(cfg_handle_t *h);

bool cfg_get_str(cfg_handle_t *h, const char *key, char *buf, size_t buflen);
bool cfg_set_str(cfg_handle_t *h, const char *key, const char *val);

bool cfg_get_u32(cfg_handle_t *h, const char *key, uint32_t *out);
bool cfg_set_u32(cfg_handle_t *h, const char *key, uint32_t val);

bool cfg_get_blob(cfg_handle_t *h, const char *key, void *buf, size_t *inout_len);
bool cfg_set_blob(cfg_handle_t *h, const char *key, const void *buf, size_t len);

bool cfg_erase(cfg_handle_t *h, const char *key);
bool cfg_commit(cfg_handle_t *h);

#ifdef __cplusplus
}
#endif

#endif
