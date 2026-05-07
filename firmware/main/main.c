#include "esp_log.h"
#include "esp_idf_version.h"

static const char *TAG = "intercom";

void app_main(void)
{
    ESP_LOGI(TAG, "Smart Intercom (SII-001) firmware boot");
    ESP_LOGI(TAG, "ESP-IDF version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Phase 0 hello-world. State machine, UI, and audio modules land in later phases.");
}
