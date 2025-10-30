#include "wifi_ap.hpp"

#include <functional>
#include <format>

#include "nvs_config.hpp"
#include "nvs_runtime.hpp"

#include "esp_log.h"
#include "esp_check.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "lwip/err.h"
#include "lwip/sys.h"


#define TAG "WAP"

using namespace std::placeholders;

#define ESP_WIFI_CHANNEL   (1)
#define MAX_STA_CONN       (4)


#define HTTP_QUERY_KEY_MAX_LEN          (64)
#define NGX_ESCAPE_URI_COMPONENT        (2)
#define NGX_UNESCAPE_URI                (1)
#define NGX_UNESCAPE_REDIRECT           (2)


WifiAP* WifiAP::_instance = nullptr;
static httpd_handle_t server = NULL;


uintptr_t   ngx_escape_uri(u_char *dst, u_char *src, size_t size, 
                            unsigned int type);
void        ngx_unescape_uri(u_char **dst, u_char **src, size_t size, 
                            unsigned int type);
uint32_t    uri_encode(char *dest, const char *src, size_t len);
void        uri_decode(char *dest, const char *src, size_t len);
static void __disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data);
static void __connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);
static httpd_handle_t __start_webserver(void);
static esp_err_t __stop_webserver(httpd_handle_t server);



static httpd_uri_t http_get_base_address = {};
static httpd_uri_t http_get_stylesheet = {};
static httpd_uri_t http_get_json = {};
static httpd_uri_t http_get_exit = {};
static httpd_uri_t http_post_config = {};
static httpd_uri_t http_post_update = {};

esp_err_t __httpGetIndex(httpd_req_t *req);
esp_err_t __httpGetStylesheet(httpd_req_t *req);
esp_err_t __httpGetJson(httpd_req_t *req);
esp_err_t __httpGetExit(httpd_req_t *req);
esp_err_t __httpPostConfig(httpd_req_t *req);
esp_err_t __httpPostUpdate(httpd_req_t *req);



WifiAP::WifiAP() {
    _bInit = false;
    _tsDisconnected = 0;
    _tsConnectionAttempt = 0;
    _state = State::kInit;
    _stateOld = _state;
    _fnEventCallback = nullptr;

    memset(&http_get_base_address, 0, sizeof(httpd_uri_t));
    memset(&http_get_stylesheet, 0, sizeof(httpd_uri_t));
    memset(&http_get_json, 0, sizeof(httpd_uri_t));
    memset(&http_get_exit, 0, sizeof(httpd_uri_t));
    memset(&http_post_config, 0, sizeof(httpd_uri_t));
    memset(&http_post_update, 0, sizeof(httpd_uri_t));
}









bool WifiAP::setup(void) {


    _bInit = true;
    return true;
}


void WifiAP::startAp() {
    if(_state < State::kConfig) {
        _state = State::kConfig;
    }
}

void WifiAP::loop(void) {
    if(!_bInit)
        return;

    if(_stateOld != _state) {
        ESP_LOGI(TAG, "State changed to: %d", static_cast<int>(_state));
        _stateOld = _state;
    }

    switch(_state) {
        case State::kInit: {
            // do nothing, wait for external call to start - startAP()
        } break;

        case State::kConfig: {
            esp_netif_t * netif = esp_netif_create_default_wifi_ap();
            wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
            ESP_ERROR_CHECK(esp_wifi_init(&cfg));

            const char* sApSSID = 
                GetCfg()->getString(INT(Cfg::Key::kAccessPointSSID));
            const char* sApPass = 
                GetCfg()->getString(INT(Cfg::Key::kAccessPointPass));


            wifi_config_t wifi_config;
            memset(&wifi_config, 0, sizeof(wifi_config));
            strcpy((char*)wifi_config.ap.ssid, sApSSID);
            strcpy((char*)wifi_config.ap.password, sApPass);
            wifi_config.ap.channel = ESP_WIFI_CHANNEL;
            wifi_config.ap.authmode = WIFI_AUTH_WPA3_PSK;
            wifi_config.ap.max_connection = MAX_STA_CONN;
            wifi_config.ap.pmf_cfg.required = true;
            wifi_config.ap.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
            if (strlen(sApPass) == 0) {
                wifi_config.ap.authmode = WIFI_AUTH_OPEN;
            }
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
            ESP_ERROR_CHECK(esp_wifi_start());
            ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s",
                    sApSSID, sApPass);

            ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, 
                IP_EVENT_AP_STAIPASSIGNED, &__connect_handler, &server));
            ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, 
                WIFI_EVENT_AP_STADISCONNECTED, &__disconnect_handler, &server));


            _state = State::kReady;
        } break;
        
        case State::kReady: {
            
        } break;
        
    }

}



esp_err_t WifiAP::httpGetIndex(httpd_req_t *req) {
    extern const char index_html_start[] asm("_binary_index_html_start");
    extern const char index_html_end[]   asm("_binary_index_html_end");    
    uint32_t len = (uint32_t)index_html_end - (uint32_t)index_html_start;
    char slen[10];
    sprintf(slen, "%ld", len);

    httpd_resp_set_status(req, HTTPD_200);
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_set_hdr(req, "Connection", "keep-alive");
    httpd_resp_set_hdr(req, "Content-Length", slen);

    httpd_resp_send(req, (const char*)((uint32_t)index_html_start), len);

    return ESP_OK;
}

esp_err_t WifiAP::httpGetJson(httpd_req_t *req) {

    std::string sJson = "{";
    char s[200];
    sprintf(s, "\"dhcp\":%d,", GetCfg()->getInt(INT(Cfg::Key::kStationDHCP))); sJson += s;
    sprintf(s, "\"event\":\"%s\",", GetCfg()->getString(INT(Cfg::Key::kEvent))); sJson += s;
    uint32_t ip = GetCfg()->getU32(INT(Cfg::Key::kStationIPv4addr));
    sprintf(s, "\"ip_address\":\"%d.%d.%d.%d\",", 
            (uint8_t)((ip >> 0)&0xFF),
            (uint8_t)((ip >> 8)&0xFF),
            (uint8_t)((ip >> 16)&0xFF),
            (uint8_t)((ip >> 24)&0xFF)); sJson += s;
    ip = GetCfg()->getU32(INT(Cfg::Key::kStationIPv4mask));            
    sprintf(s, "\"ip_mask\":\"%d.%d.%d.%d\",", 
            (uint8_t)((ip >> 0)&0xFF),
            (uint8_t)((ip >> 8)&0xFF),
            (uint8_t)((ip >> 16)&0xFF),
            (uint8_t)((ip >> 24)&0xFF)); sJson += s;
    ip = GetCfg()->getU32(INT(Cfg::Key::kStationGateway));            
    sprintf(s, "\"gateway\":\"%d.%d.%d.%d\",", 
            (uint8_t)((ip >> 0)&0xFF),
            (uint8_t)((ip >> 8)&0xFF),
            (uint8_t)((ip >> 16)&0xFF),
            (uint8_t)((ip >> 24)&0xFF)); sJson += s;
    sprintf(s, "\"mqtt\":\"%s\",", GetCfg()->getString(INT(Cfg::Key::kMqttServer))); sJson += s;
    sprintf(s, "\"mqtt_port\":\"%d\",", GetCfg()->getInt(INT(Cfg::Key::kMqttPort))); sJson += s;
    sprintf(s, "\"mqtt_user\":\"%s\",", GetCfg()->getString(INT(Cfg::Key::kMqttUser))); sJson += s;
    sprintf(s, "\"mqtt_user_pass\":\"%s\",", GetCfg()->getString(INT(Cfg::Key::kMqttUserPass))); sJson += s;
    sprintf(s, "\"relay_time\":\"%d\",", GetCfg()->getInt(INT(Cfg::Key::kRelayTime))); sJson += s;
    sprintf(s, "\"sound\":\"%d\",", GetCfg()->getInt(INT(Cfg::Key::kSound))); sJson += s;
    sprintf(s, "\"name\":\"%s\",", GetCfg()->getString(INT(Cfg::Key::kDeviceName))); sJson += s;
    sprintf(s, "\"ssid\":\"%s\",", GetCfg()->getString(INT(Cfg::Key::kStationSSID))); sJson += s;
    sprintf(s, "\"pass\":\"%s\",", GetCfg()->getString(INT(Cfg::Key::kStationPass))); sJson += s;
    sprintf(s, "\"apssid\":\"%s\",", GetCfg()->getString(INT(Cfg::Key::kAccessPointSSID))); sJson += s;
    sprintf(s, "\"appass\":\"%s\",", GetCfg()->getString(INT(Cfg::Key::kAccessPointPass))); sJson += s;
    sprintf(s, "\"type\":\"%s\",", GetCfg()->getString(INT(Cfg::Key::kDeviceType))); sJson += s;
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);        // Get address from hardware eFuses
    sprintf(s, "\"device_mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\",", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); sJson += s;
    sprintf(s, "\"hotel_code\":\"%d\",", GetCfg()->getInt(INT(Cfg::Key::kHotelCode))); sJson += s;
    sprintf(s, "\"room\":\"%d\",", GetCfg()->getInt(INT(Cfg::Key::kRoomNumber))); sJson += s;
    sprintf(s, "\"group\":\"%llu\",", GetCfg()->getU64(INT(Cfg::Key::kAccessGroup))); sJson += s;
    sprintf(s, "\"guest_count\":\"%lu\",", GetRt()->getU32(INT(Rt::Key::kCounterGuest))); sJson += s;
    sprintf(s, "\"staff_count\":\"%lu\",", GetRt()->getU32(INT(Rt::Key::kCounterStaff))); sJson += s;
    sprintf(s, "\"master_count\":\"%lu\",", GetRt()->getU32(INT(Rt::Key::kCounterMaster))); sJson += s;
    ip = GetCfg()->getU32(INT(Cfg::Key::kStationPrimaryDns));            
    sprintf(s, "\"primary_dns\":\"%d.%d.%d.%d\",", 
            (uint8_t)((ip >> 0)&0xFF),
            (uint8_t)((ip >> 8)&0xFF),
            (uint8_t)((ip >> 16)&0xFF),
            (uint8_t)((ip >> 24)&0xFF)); sJson += s;
    ip = GetCfg()->getU32(INT(Cfg::Key::kStationSecondaryDns));            
    sprintf(s, "\"secondary_dns\":\"%d.%d.%d.%d\",", 
            (uint8_t)((ip >> 0)&0xFF),
            (uint8_t)((ip >> 8)&0xFF),
            (uint8_t)((ip >> 16)&0xFF),
            (uint8_t)((ip >> 24)&0xFF)); sJson += s;
    sprintf(s, "\"keyA\":\"FFFFFFFFFFFF\","); sJson += s;
    sprintf(s, "\"update_code\":\"%s\"", GetCfg()->getString(INT(Cfg::Key::kKeyUpdate))); sJson += s;
    sJson += "}";

    uint32_t len = sJson.size();
    char slen[10];
    sprintf(slen, "%ld", len);

    httpd_resp_set_status(req, HTTPD_200);
    httpd_resp_set_type(req, "application/json; charset=utf-8");
    httpd_resp_set_hdr(req, "Connection", "keep-alive");
    httpd_resp_set_hdr(req, "Content-Length", slen);

    httpd_resp_send(req, (const char*)((uint32_t)sJson.c_str()), len);

    return ESP_OK;
}

esp_err_t WifiAP::httpGetStylesheet(httpd_req_t *req) {
    extern const char style_css_start[] asm("_binary_style_css_start");
    extern const char style_css_end[]   asm("_binary_style_css_end");    
    uint32_t len = (uint32_t)style_css_end - (uint32_t)style_css_start;
    char slen[10];
    sprintf(slen, "%ld", len);

    httpd_resp_set_status(req, HTTPD_200);
    httpd_resp_set_type(req, "text/css; charset=utf-8");
    httpd_resp_set_hdr(req, "Connection", "keep-alive");
    httpd_resp_set_hdr(req, "Content-Length", slen);

    httpd_resp_send(req, (const char*)((uint32_t)style_css_start), len);

    return ESP_OK;
}

esp_err_t WifiAP::httpGetExit(httpd_req_t *req) {
    httpd_resp_set_status(req, HTTPD_200);
    httpd_resp_send(req, nullptr, 0);

    execCallback(InsideEvent::kReboot, nullptr, 0);

    return ESP_OK;
}

esp_err_t WifiAP::httpPostConfig(httpd_req_t *req) {

    return ESP_OK;
}

esp_err_t WifiAP::httpPostUpdate(httpd_req_t *req) {
    httpd_resp_set_status(req, HTTPD_404);
    return ESP_OK;
}


void WifiAP::execCallback(InsideEvent event, void* pdata, uint16_t len) {
    if(_fnEventCallback != nullptr) {
        _fnEventCallback(event, pdata, len);
    }
}



static void __disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (__stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

static void __connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = __start_webserver();
    }
}


static httpd_handle_t __start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");

        http_get_base_address.uri = "/";
        http_get_base_address.method = HTTP_GET;
        http_get_base_address.handler = __httpGetIndex;
        http_get_base_address.user_ctx = (void*) NULL;
        httpd_register_uri_handler(server, &http_get_base_address);

        http_get_stylesheet.uri = "/style.css";
        http_get_stylesheet.method = HTTP_GET;
        http_get_stylesheet.handler = __httpGetStylesheet;
        http_get_stylesheet.user_ctx = (void*) NULL;
        httpd_register_uri_handler(server, &http_get_stylesheet);

        http_get_json.uri = "/config.json";
        http_get_json.method = HTTP_GET;
        http_get_json.handler = __httpGetJson;
        http_get_json.user_ctx = (void*) NULL;
        httpd_register_uri_handler(server, &http_get_json);

        http_get_exit.uri = "/exit";
        http_get_exit.method = HTTP_GET;
        http_get_exit.handler = __httpGetExit;
        http_get_exit.user_ctx = (void*) NULL;
        httpd_register_uri_handler(server, &http_get_exit);

        http_post_config.uri = "/config";
        http_post_config.method = HTTP_POST;
        http_post_config.handler = __httpPostConfig;
        http_post_config.user_ctx = (void*) NULL;
        httpd_register_uri_handler(server, &http_post_config);

        http_post_update.uri = "/update";
        http_post_update.method = HTTP_POST;
        http_post_update.handler = __httpPostUpdate;
        http_post_update.user_ctx = (void*) NULL;
        httpd_register_uri_handler(server, &http_post_update);

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}



static esp_err_t __stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}



esp_err_t __httpGetIndex(httpd_req_t *req) {
    return WifiAP::getInstance().httpGetIndex(req);
}

esp_err_t __httpGetJson(httpd_req_t *req) {
    return WifiAP::getInstance().httpGetJson(req);
}

esp_err_t __httpGetStylesheet(httpd_req_t *req) {
    return WifiAP::getInstance().httpGetStylesheet(req);
}

esp_err_t __httpGetExit(httpd_req_t *req) {
    return WifiAP::getInstance().httpGetExit(req);
}

esp_err_t __httpPostConfig(httpd_req_t *req) {
    return WifiAP::getInstance().httpPostConfig(req);
}

esp_err_t __httpPostUpdate(httpd_req_t *req) {
    return WifiAP::getInstance().httpPostUpdate(req);
}



uintptr_t ngx_escape_uri(u_char *dst, u_char *src, size_t size, 
                                unsigned int type)
{
    unsigned int      n;
    uint32_t       *escape;
    static u_char   hex[] = "0123456789ABCDEF";

    /*
     * Per RFC 3986 only the following chars are allowed in URIs unescaped:
     *
     * unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
     * gen-delims    = ":" / "/" / "?" / "#" / "[" / "]" / "@"
     * sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
     *               / "*" / "+" / "," / ";" / "="
     *
     * And "%" can appear as a part of escaping itself.  The following
     * characters are not allowed and need to be escaped: %00-%1F, %7F-%FF,
     * " ", """, "<", ">", "\", "^", "`", "{", "|", "}".
     */

    /* " ", "#", "%", "?", not allowed */

    static uint32_t   uri[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

        /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0xd000002d, /* 1101 0000 0000 0000  0000 0000 0010 1101 */

        /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x50000000, /* 0101 0000 0000 0000  0000 0000 0000 0000 */

        /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0xb8000001, /* 1011 1000 0000 0000  0000 0000 0000 0001 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

    /* " ", "#", "%", "&", "+", ";", "?", not allowed */

    static uint32_t   args[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

        /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0xd800086d, /* 1101 1000 0000 0000  0000 1000 0110 1101 */

        /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x50000000, /* 0101 0000 0000 0000  0000 0000 0000 0000 */

        /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0xb8000001, /* 1011 1000 0000 0000  0000 0000 0000 0001 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

    /* not ALPHA, DIGIT, "-", ".", "_", "~" */

    static uint32_t   uri_component[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

        /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0xfc009fff, /* 1111 1100 0000 0000  1001 1111 1111 1111 */

        /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x78000001, /* 0111 1000 0000 0000  0000 0000 0000 0001 */

        /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0xb8000001, /* 1011 1000 0000 0000  0000 0000 0000 0001 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

    /* " ", "#", """, "%", "'", not allowed */

    static uint32_t   html[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

        /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x500000ad, /* 0101 0000 0000 0000  0000 0000 1010 1101 */

        /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x50000000, /* 0101 0000 0000 0000  0000 0000 0000 0000 */

        /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0xb8000001, /* 1011 1000 0000 0000  0000 0000 0000 0001 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

    /* " ", """, "'", not allowed */

    static uint32_t   refresh[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

        /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x50000085, /* 0101 0000 0000 0000  0000 0000 1000 0101 */

        /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x50000000, /* 0101 0000 0000 0000  0000 0000 0000 0000 */

        /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0xd8000001, /* 1011 1000 0000 0000  0000 0000 0000 0001 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

    /* " ", "%", %00-%1F */

    static uint32_t   memcached[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

        /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x00000021, /* 0000 0000 0000 0000  0000 0000 0010 0001 */

        /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

        /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
    };

    /* mail_auth is the same as memcached */

    static uint32_t  *map[] =
    { uri, args, uri_component, html, refresh, memcached, memcached };


    escape = map[type];

    if (dst == NULL) {

        /* find the number of the characters to be escaped */

        n = 0;

        while (size) {
            if (escape[*src >> 5] & (1U << (*src & 0x1f))) {
                n++;
            }
            src++;
            size--;
        }

        return (uintptr_t) n;
    }

    while (size) {
        if (escape[*src >> 5] & (1U << (*src & 0x1f))) {
            *dst++ = '%';
            *dst++ = hex[*src >> 4];
            *dst++ = hex[*src & 0xf];
            src++;

        } else {
            *dst++ = *src++;
        }
        size--;
    }

    return (uintptr_t) dst;
}



void ngx_unescape_uri(u_char **dst, u_char **src, size_t size, 
                        unsigned int type)
{
    u_char  *d, *s, ch, c, decoded;
    enum {
        sw_usual = 0,
        sw_quoted,
        sw_quoted_second
    } state;

    d = *dst;
    s = *src;

    state = sw_usual;
    decoded = 0;

    while (size--) {

        ch = *s++;

        switch (state) {
        case sw_usual:
            if (ch == '?'
                    && (type & (NGX_UNESCAPE_URI | NGX_UNESCAPE_REDIRECT))) {
                *d++ = ch;
                goto done;
            }

            if (ch == '%') {
                state = sw_quoted;
                break;
            }

            *d++ = ch;
            break;

        case sw_quoted:

            if (ch >= '0' && ch <= '9') {
                decoded = (u_char) (ch - '0');
                state = sw_quoted_second;
                break;
            }

            c = (u_char) (ch | 0x20);
            if (c >= 'a' && c <= 'f') {
                decoded = (u_char) (c - 'a' + 10);
                state = sw_quoted_second;
                break;
            }

            /* the invalid quoted character */

            state = sw_usual;

            *d++ = ch;

            break;

        case sw_quoted_second:

            state = sw_usual;

            if (ch >= '0' && ch <= '9') {
                ch = (u_char) ((decoded << 4) + (ch - '0'));

                if (type & NGX_UNESCAPE_REDIRECT) {
                    if (ch > '%' && ch < 0x7f) {
                        *d++ = ch;
                        break;
                    }

                    *d++ = '%'; *d++ = *(s - 2); *d++ = *(s - 1);

                    break;
                }

                *d++ = ch;

                break;
            }

            c = (u_char) (ch | 0x20);
            if (c >= 'a' && c <= 'f') {
                ch = (u_char) ((decoded << 4) + (c - 'a') + 10);

                if (type & NGX_UNESCAPE_URI) {
                    if (ch == '?') {
                        *d++ = ch;
                        goto done;
                    }

                    *d++ = ch;
                    break;
                }

                if (type & NGX_UNESCAPE_REDIRECT) {
                    if (ch == '?') {
                        *d++ = ch;
                        goto done;
                    }

                    if (ch > '%' && ch < 0x7f) {
                        *d++ = ch;
                        break;
                    }

                    *d++ = '%'; *d++ = *(s - 2); *d++ = *(s - 1);
                    break;
                }

                *d++ = ch;

                break;
            }

            /* the invalid quoted character */

            break;
        }
    }

done:

    *dst = d;
    *src = s;
}



uint32_t uri_encode(char *dest, const char *src, size_t len)
{
    if (!src || !dest) {
        return 0;
    }

    uintptr_t ret = ngx_escape_uri((unsigned char *)dest, (unsigned char *)src, 
                len, NGX_ESCAPE_URI_COMPONENT);
    return (uint32_t)(ret - (uintptr_t)dest);
}


void uri_decode(char *dest, const char *src, size_t len)
{
    if (!src || !dest) {
        return;
    }

    unsigned char *src_ptr = (unsigned char *)src;
    unsigned char *dst_ptr = (unsigned char *)dest;
    ngx_unescape_uri(&dst_ptr, &src_ptr, len, NGX_UNESCAPE_URI);
}



