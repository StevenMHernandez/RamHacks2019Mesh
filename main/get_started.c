// Copyright 2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "mdf_common.h"
#include "mwifi.h"

#include "driver/gpio.h"

// #define MEMORY_DEBUG

static const char *TAG = "get_started";
#define BUTTON_GPIO 12

int blink_gpio = 21;

long int last_warning_message = 0;
bool is_blinking = false;

static void blink_task(void *arg) {
    mdf_err_t ret = MDF_OK;
    char *data = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size = MWIFI_PAYLOAD_LEN;
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};
    mwifi_data_type_t data_type = {0};

    for (;;) {
        struct timeval currentTime;
        gettimeofday(&currentTime, NULL);

        if (last_warning_message > 0 && last_warning_message + 3 > currentTime.tv_sec) {
            /* Blink on (output high) */
            gpio_set_level(blink_gpio, 1);
            vTaskDelay(20 / portTICK_PERIOD_MS);
            /* Blink off (output low) */
            gpio_set_level(blink_gpio, 0);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            is_blinking = true;
        } else {
            if (is_blinking) {
                is_blinking = false;

                uint8_t sta_mac2[MWIFI_ADDR_LEN] = {0};
                esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac2);
                printf("Ended: %02X:%02X:%02X:%02X:%02X:%02X\n", sta_mac2[0], sta_mac2[1], sta_mac2[2], sta_mac2[3], sta_mac2[4], sta_mac2[5]);

                wifi_sta_list_t wifi_sta_list = {0x0};
                esp_wifi_ap_get_sta_list(&wifi_sta_list);
                for (int i = 0; i < wifi_sta_list.num; i++) {
                    wifi_sta_info_t d = wifi_sta_list.sta[i];
                    printf("Ended: %02X:%02X:%02X:%02X:%02X:%02X\n", d.mac[0], d.mac[1], d.mac[2], d.mac[3], d.mac[4], d.mac[5]);
                    ret = mwifi_root_write(wifi_sta_list.sta[i].mac, 1, &data_type, data, size, true);
                }
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

static void root_task(void *arg) {
    mdf_err_t ret = MDF_OK;
    char *data = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size = MWIFI_PAYLOAD_LEN;
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};
    mwifi_data_type_t data_type = {0};

    MDF_LOGI("Root is running");

    for (int i = 0;; ++i) {
        if (!mwifi_is_started()) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        size = sprintf(data, "(%d) Hello node!", i);

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_root_read", mdf_err_to_name(ret));
        MDF_LOGI("Root receive, addr: "
        MACSTR
        ", size: %d, data: %s", MAC2STR(src_addr), size, data);

        struct timeval currentTime;
        gettimeofday(&currentTime, NULL);
        last_warning_message = currentTime.tv_sec;




        uint8_t primary = 0;
        wifi_second_chan_t second = 0;
        mesh_addr_t parent_bssid = {0};
        uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
        mesh_assoc_t mesh_assoc = {0x0};
        wifi_sta_list_t wifi_sta_list = {0x0};

        esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
        esp_wifi_ap_get_sta_list(&wifi_sta_list);
        esp_wifi_get_channel(&primary, &second);
        esp_wifi_vnd_mesh_get(&mesh_assoc);
        esp_mesh_get_parent_bssid(&parent_bssid);

//        printf("Finished: %")


//        MDF_LOGI("System information, channel: %d, layer: %d, self mac: "
//        MACSTR
//        ", parent bssid: "
//        MACSTR
//        ", parent rssi: %d, node num: %d, free heap: %u", primary,
//                esp_mesh_get_layer(), MAC2STR(sta_mac), MAC2STR(parent_bssid.addr),
//                mesh_assoc.rssi, esp_mesh_get_total_node_num(), esp_get_free_heap_size());

        wifi_sta_info_t d = wifi_sta_list.sta[i];
        ret = mwifi_root_write(src_addr, 1, &data_type, data, size, true);
//        ret = mwifi_root_write(&wifi_sta_list.sta[i], wifi_sta_list.num, &data_type, data, size, true);




        uint8_t sta_mac2[MWIFI_ADDR_LEN] = {0};
        esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac2);
        printf("Started: %02X:%02X:%02X:%02X:%02X:%02X\n", sta_mac2[0], sta_mac2[1], sta_mac2[2], sta_mac2[3], sta_mac2[4], sta_mac2[5]);

        for (int i = 0; i < wifi_sta_list.num; i++) {
            MDF_LOGI("Root send to other addr: "
            MACSTR
            ", size: %d, data: %s", MAC2STR(wifi_sta_list.sta[i].mac), size, data);
            wifi_sta_info_t d = wifi_sta_list.sta[i];
            printf("Started: %02X:%02X:%02X:%02X:%02X:%02X\n", d.mac[0], d.mac[1], d.mac[2], d.mac[3], d.mac[4], d.mac[5]);
            ret = mwifi_root_write(wifi_sta_list.sta[i].mac, 1, &data_type, data, size, true);
        }







        MDF_ERROR_CONTINUE(ret != MDF_OK, "mwifi_root_recv, ret: %x", ret);
        MDF_LOGI("Root send, addr: "
        MACSTR
        ", size: %d, data: %s", MAC2STR(src_addr), size, data);
    }

    MDF_LOGW("Root is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

static void node_read_task(void *arg) {
    mdf_err_t ret = MDF_OK;
    char *data = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type = {0x0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};

    MDF_LOGI("Note read task is running");

    for (;;) {
        if (!mwifi_is_connected()) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "mwifi_read, ret: %x", ret);
        MDF_LOGI("Node receive, addr: "
        MACSTR
        ", size: %d, data: %s", MAC2STR(src_addr), size, data);



        struct timeval currentTime;
        gettimeofday(&currentTime, NULL);
        last_warning_message = currentTime.tv_sec;
    }

    MDF_LOGW("Note read task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

bool last_button_value = false;

void node_write_task(void *arg) {
    mdf_err_t ret = MDF_OK;
    int count = 0;
    size_t size = 0;
    char *data = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    mwifi_data_type_t data_type = {0x0};

    MDF_LOGI("Node write task is running");

    for (;;) {
        if (!mwifi_is_connected()) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        if (last_button_value != gpio_get_level(BUTTON_GPIO)) {
            last_button_value = gpio_get_level(BUTTON_GPIO);
            if (!gpio_get_level(BUTTON_GPIO)) {
                printf("Button pressed. Sending warning signal!\n");
                size = sprintf(data, "(%d) !!!", count++);
                ret = mwifi_write(NULL, &data_type, data, size, true);
                MDF_ERROR_CONTINUE(ret != MDF_OK, "mwifi_write, ret: %x", ret);
            }
        }

        vTaskDelay(100 / portTICK_RATE_MS);
//        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    MDF_LOGW("Node write task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

/**
 * @brief Timed printing system information
 */
static void print_system_info_timercb(void *timer) {
    uint8_t primary = 0;
    wifi_second_chan_t second = 0;
    mesh_addr_t parent_bssid = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
    mesh_assoc_t mesh_assoc = {0x0};
    wifi_sta_list_t wifi_sta_list = {0x0};

    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    esp_wifi_get_channel(&primary, &second);
    esp_wifi_vnd_mesh_get(&mesh_assoc);
    esp_mesh_get_parent_bssid(&parent_bssid);

    MDF_LOGI("System information, channel: %d, layer: %d, self mac: "
    MACSTR
    ", parent bssid: "
    MACSTR
    ", parent rssi: %d, node num: %d, free heap: %u", primary,
            esp_mesh_get_layer(), MAC2STR(sta_mac), MAC2STR(parent_bssid.addr),
            mesh_assoc.rssi, esp_mesh_get_total_node_num(), esp_get_free_heap_size());

    uint8_t sta_mac2[MWIFI_ADDR_LEN] = {0};
    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac2);
    printf("Alive: %02X:%02X:%02X:%02X:%02X:%02X\n", sta_mac2[0], sta_mac2[1], sta_mac2[2], sta_mac2[3], sta_mac2[4], sta_mac2[5]);

    for (int i = 0; i < wifi_sta_list.num; i++) {
        MDF_LOGI("Child mac: "
        MACSTR, MAC2STR(wifi_sta_list.sta[i].mac));

        wifi_sta_info_t d = wifi_sta_list.sta[i];
        printf("Alive: %02X:%02X:%02X:%02X:%02X:%02X\n", d.mac[0], d.mac[1], d.mac[2], d.mac[3], d.mac[4], d.mac[5]);
    }

#ifdef MEMORY_DEBUG
    if (!heap_caps_check_integrity_all(true)) {
        MDF_LOGE("At least one heap is corrupt");
    }

    mdf_mem_print_heap();
    mdf_mem_print_record();
#endif /**< MEMORY_DEBUG */
}

static mdf_err_t wifi_init() {
    mdf_err_t ret = nvs_flash_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        MDF_ERROR_ASSERT(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    MDF_ERROR_ASSERT(ret);

    tcpip_adapter_init();
    MDF_ERROR_ASSERT(esp_event_loop_init(NULL, NULL));
    MDF_ERROR_ASSERT(esp_wifi_init(&cfg));
    MDF_ERROR_ASSERT(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    MDF_ERROR_ASSERT(esp_wifi_set_mode(WIFI_MODE_STA));
    MDF_ERROR_ASSERT(esp_wifi_set_ps(WIFI_PS_NONE));
    MDF_ERROR_ASSERT(esp_mesh_set_6m_rate(false));
    MDF_ERROR_ASSERT(esp_wifi_start());

    return MDF_OK;
}

/**
 * @brief All module events will be sent to this task in esp-mdf
 *
 * @Note:
 *     1. Do not block or lengthy operations in the callback function.
 *     2. Do not consume a lot of memory in the callback function.
 *        The task memory of the callback function is only 4KB.
 */
static mdf_err_t event_loop_cb(mdf_event_loop_t event, void *ctx) {
    MDF_LOGI("event_loop_cb, event: %d", event);

    switch (event) {
        case MDF_EVENT_MWIFI_STARTED:
            MDF_LOGI("MESH is started");
            break;

        case MDF_EVENT_MWIFI_PARENT_CONNECTED:
            MDF_LOGI("Parent is connected on station interface");
            break;

        case MDF_EVENT_MWIFI_PARENT_DISCONNECTED:
            MDF_LOGI("Parent is disconnected on station interface");
            break;

        default:
            break;
    }

    return MDF_OK;
}

void app_main() {
    mwifi_init_config_t cfg = MWIFI_INIT_CONFIG_DEFAULT();
    mwifi_config_t config = {
            .channel   = CONFIG_MESH_CHANNEL,
            .mesh_id   = CONFIG_MESH_ID,
            .mesh_type = CONFIG_DEVICE_TYPE,
    };
    config.mesh_type = MESH_ROOT;
//    config.mesh_type = MESH_NODE;

    blink_gpio = config.mesh_type == MESH_ROOT ? 21 : 2;

    /**
     * @brief Set the log level for serial port printing.
     */
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    /**
     * @brief Initialize wifi mesh.
     */
    MDF_ERROR_ASSERT(mdf_event_loop_init(event_loop_cb));
    MDF_ERROR_ASSERT(wifi_init());
    MDF_ERROR_ASSERT(mwifi_init(&cfg));
    MDF_ERROR_ASSERT(mwifi_set_config(&config));
    MDF_ERROR_ASSERT(mwifi_start());

    /**
     * @brief Data transfer between wifi mesh devices
     */
    if (config.mesh_type == MESH_ROOT) {
        xTaskCreate(root_task, "root_task", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    } else {
        xTaskCreate(node_write_task, "node_write_task", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    }

    xTaskCreate(node_read_task, "node_read_task", 4 * 1024,
                NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    xTaskCreate(blink_task, "blink_task", 4 * 1024,
                NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);

    TimerHandle_t timer = xTimerCreate("print_system_info", 10000 / portTICK_RATE_MS,
                                       true, NULL, print_system_info_timercb);
    xTimerStart(timer, 0);


    gpio_pad_select_gpio(blink_gpio);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(blink_gpio, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

}
