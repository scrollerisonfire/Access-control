#include "arduino_wrapper.hpp"
#include "esp_timer.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/uart.h>

#include "board.hpp"




void SPIClass::begin(gpio_num_t chipSelectPin, gpio_num_t reset, gpio_num_t mosi, gpio_num_t miso, gpio_num_t clk)
{
	gpio_config_t io_conf = {};

	// disable interrupt
	io_conf.intr_type = GPIO_INTR_DISABLE;

	// set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;

	// bit mask of the pins that you want to set,e.g.GPIO18/19
	io_conf.pin_bit_mask = (1 << chipSelectPin) | (1 << reset);
	// disable pull-down mode
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	// disable pull-up mode
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	// configure GPIO with the given settings
	gpio_config(&io_conf);

	gpio_set_level(chipSelectPin, LOW);
	gpio_set_level(reset, LOW);

	vTaskDelay(100 / portTICK_PERIOD_MS);

	spi_bus_config_t buscfg;
	memset(&buscfg, 0, sizeof(spi_bus_config_t));

	buscfg.miso_io_num = miso;
	buscfg.mosi_io_num = mosi;
	buscfg.sclk_io_num = clk;
	buscfg.quadwp_io_num = -1;
	buscfg.quadhd_io_num = -1;
	
	buscfg.data2_io_num = -1;   ///< GPIO pin for spi data2 signal in quad/octal mode, or -1 if not used.
    buscfg.data3_io_num = -1;   ///< GPIO pin for spi data3 signal in quad/octal mode, or -1 if not used.
    buscfg.data4_io_num = -1;   ///< GPIO pin for spi data3 signal in quad/octal mode, or -1 if not used.
    buscfg.data5_io_num = -1;   ///< GPIO pin for spi data3 signal in quad/octal mode, or -1 if not used.
    buscfg.data6_io_num = -1;   ///< GPIO pin for spi data3 signal in quad/octal mode, or -1 if not used.
    buscfg.data7_io_num = -1;   ///< GPIO pin for spi data3 signal in quad/octal mode, or -1 if not used.
	
	buscfg.max_transfer_sz = 128;

	spi_device_interface_config_t devcfg;
	memset(&devcfg, 0, sizeof(spi_device_interface_config_t));

	devcfg.clock_speed_hz = 10 * 1000 * 1000; // Clock out at 10 MHz
	devcfg.mode = 0;						 // SPI mode 0
	devcfg.spics_io_num = -1;				 // CS pin
	devcfg.queue_size =
		7; // We want to be able to queue 7 transactions at a time
	devcfg.pre_cb = nullptr; // Specify pre-transfer callback to handle D/C line

	// Initialize the SPI bus
	spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_DISABLED);
	spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
}




void SPIClass::beginTransaction(SPISettings settings) {
    // do nothing
}


void SPIClass::endTransaction(void) {
    // do nothing
}


uint8_t SPIClass::transfer(uint8_t data) {

    esp_err_t returnCode;
    spi_transaction_t t = {};
    
    uint8_t result;

    t.length = 8;                   //Command is 8 bits
    t.tx_buffer = &data;            //The data is the cmd itself
    t.rx_buffer = &result;            //The data is the cmd itself
    t.user = (void*)0;              //D/C needs to be set to 0

    returnCode = spi_device_polling_transmit(spi, &t); //Transmit!
    
    if(returnCode == ESP_OK)
    {
		return result;
	}
	return 0;    
}


void digitalWrite(int pin, int level)
{
    gpio_set_level((gpio_num_t)pin, level);
}

int digitalRead(uint8_t pin) {
    return gpio_get_level((gpio_num_t)pin);
}


uint32_t millis() {
    return ((uint64_t) (esp_timer_get_time() / 1000ULL));
}

void delayMicroseconds(uint32_t us) {
    uint64_t start = esp_timer_get_time();
    uint64_t end = start + us;
    while(esp_timer_get_time() < end);
}

void delay(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void yield(void) {
    vTaskDelay(1);
}

void pinMode(uint8_t pin, uint8_t mode) {
    Board::pinMode(pin, (gpio_mode_t)mode);
}










// size_t Print::printf(const char * format, ...)  __attribute__ ((format (::printf, 2, 3)));
// size_t Print::print(const __FlashStringHelper *ifsh) { return print(reinterpret_cast<const char *>(ifsh)); }
size_t Print::print(const String &s) {
    return ::printf("%s", s.c_str());
}
size_t Print::print(const char* s) {
    return ::printf("%s", s);
}
size_t Print::print(char ll) {
    return ::printf("%c", ll);
}
size_t Print::print(unsigned char ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%dd", padding);
    return ::printf(fmt, ll);
}
size_t Print::print(int ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%dd", padding);
    return ::printf(fmt, ll);
}
size_t Print::print(unsigned int ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%du", padding);
    return ::printf(fmt, ll);
}
size_t Print::print(long ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%dl", padding);
    return ::printf(fmt, ll);
}
size_t Print::print(unsigned long ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%dlu", padding);
    return ::printf(fmt, ll);
}
size_t Print::print(long long ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%dll", padding);
    return ::printf(fmt, ll);
}
size_t Print::print(unsigned long long ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%dllu", padding);
    return ::printf(fmt, ll);
}
size_t Print::print(double f, int prec) {
    char fmt[8] = {};
    sprintf(fmt, "%%.%df", prec);
    return ::printf(fmt, f);
}
size_t Print::print(struct tm * timeinfo, const char * format) {
    return ::printf("timeinfo:not_imlemented");
}

// size_t Print::println(const __FlashStringHelper *ifsh) { return println(reinterpret_cast<const char *>(ifsh)); }
size_t Print::println(const String &s) {
    return ::printf("%s\n", s.c_str());
}
size_t Print::println(const char* s) {
    return ::printf("%s\n", s);
}
size_t Print::println(char ll) {
    return ::printf("%c\n", ll);
}
size_t Print::println(unsigned char ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%dd\n", padding);
    return ::printf(fmt, ll);
}
size_t Print::println(int ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%dd\n", padding);
    return ::printf(fmt, ll);
}
size_t Print::println(unsigned int ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%du\n", padding);
    return ::printf(fmt, ll);
}
size_t Print::println(long ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%dl\n", padding);
    return ::printf(fmt, ll);
}
size_t Print::println(unsigned long ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%dlu\n", padding);
    return ::printf(fmt, ll);
}
size_t Print::println(long long ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%dll\n", padding);
    return ::printf(fmt, ll);
}
size_t Print::println(unsigned long long ll, int padding) {
    char fmt[8] = {};
    sprintf(fmt, "%%%dllu\n", padding);
    return ::printf(fmt, ll);
}
size_t Print::println(double f, int prec) {
    char fmt[8] = {};
    sprintf(fmt, "%%.%df\n", prec);
    return ::printf(fmt, f);
}
size_t Print::println(struct tm * timeinfo, const char * format) {
    return ::printf("timeinfo:not_imlemented\n");
}
size_t Print::println(void) {
    return ::printf("\n");
}







size_t HardwareSerial::write(uint8_t c)
{
    return uart_write_bytes(_port, &c, 1);
}

size_t HardwareSerial::write(const uint8_t *buffer, size_t size) 
{
    return uart_write_bytes(_port, buffer, size);
}


HardwareSerial::HardwareSerial(uart_port_t port) {
    _port = port;

    uart_config_t uart_config = {
        .baud_rate = 10000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        /*.rx_flow_ctrl_thresh = 122,*/
        .source_clk = UART_SCLK_DEFAULT,
    };

    
    ESP_ERROR_CHECK(uart_param_config(_port, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(_port, TX_CAM_PIN, RX_CAM_PIN, -1, -1));
    const int uart_buffer_size = (1024 * 2);
    QueueHandle_t uart_queue;
    // Install UART driver using an event queue here
    ESP_ERROR_CHECK(uart_driver_install(_port, uart_buffer_size, \
                                            uart_buffer_size, 10, &uart_queue, 0));

}


int HardwareSerial::available(void)
{
    size_t size;
    esp_err_t err = uart_get_buffered_data_len(_port, &size);
    if(err == ESP_OK)
        return size;
    return 0;
}


int HardwareSerial::peek(void)
{
    if (available()) {
        return read();
    }
    return -1;
}


int HardwareSerial::read(void)
{
    uint8_t c = 0;
    if (uart_read_bytes(_port, &c, 1, 0) == 1) {
        return c;
    } else {
        return -1;
    }
}


size_t HardwareSerial::read(uint8_t *buffer, size_t size)
{
    return uart_read_bytes(_port, buffer, size, 0);
}









Print Serial;
SPIClass SPI;


HardwareSerial Serial2(UART_NUM_1);