#include "_wificonnect.h"
#include "_temp.h"

#include <stdio.h>
#include <string.h>

#include "esp_wifi.h"
#include "mqtt_client.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "esp_sntp.h"
#include "time.h"



#define TAG "WiFiConnect"

#define EXAMPLE_ESP_WIFI_SSID ""
#define EXAMPLE_ESP_WIFI_PASS ""
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_H2E
#define BROKER_URI ""
#define MQTT_PORT 1902

bool connected = false;

esp_mqtt_client_handle_t client;

esp_mqtt_client_config_t mqtt_config = {
    .broker = {
        .address = {
            .uri = BROKER_URI,
            .hostname = "", 
            .port = MQTT_PORT,         
        },
    },
    .credentials = {
        .username = "user", 
        .authentication = {
            .password = "", 
        },
    },
    //.client_id = "your_client_id", 
};


void publish_message()
{
    esp_err_t err;
    const char *message = "Hello, MQTT!";
    err = esp_mqtt_client_publish(client, "/topic/test", message, 0, 1, 0); 
    printf ("esp_mqtt_client_publish = %d\n",err);
}

void mqtt_app_start(void)
{
    esp_err_t err;
    client = esp_mqtt_client_init(&mqtt_config);
    err = esp_mqtt_client_start(client);
    printf ("esp_mqtt_client_start = %d\n",err);

    publish_message();
}



static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting WIFI_EVENT_STA_START ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected WIFI_EVENT_STA_CONNECTED ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection WIFI_EVENT_STA_DISCONNECTED ... \n");
        connected = false;
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        connected = true;
        break;
    default:
        break;
    }
}

void obtain_time()
{
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    // Wait for time synchronization
    printf("Waiting for time synchronization...\n");
    while (!sntp_get_sync_status())
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    time_t timestamp = 0;
    struct tm timeinfo;
    time(&timestamp);
    setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);
    tzset();
    localtime_r(&timestamp, &timeinfo);
    
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    printf("Current time: %s\n", strftime_buf);
}

void wifi_connection()
{
    nvs_flash_init();
    esp_netif_init();                    
    esp_event_loop_create_default();     
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); 
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    
    wifi_config_t wifi_configuration = { };
    memcpy(wifi_configuration.sta.ssid, EXAMPLE_ESP_WIFI_SSID, sizeof(EXAMPLE_ESP_WIFI_SSID));
    memcpy(wifi_configuration.sta.password, EXAMPLE_ESP_WIFI_PASS, sizeof(EXAMPLE_ESP_WIFI_PASS));
    esp_wifi_set_config(WIFI_IF_STA, &wifi_configuration);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_wifi_connect();

    while (!connected)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    obtain_time();
    mqtt_app_start(); 
}
