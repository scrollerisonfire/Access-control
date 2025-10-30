#ifndef __ARDUINO_WRAPPER_HPP__
#define __ARDUINO_WRAPPER_HPP__

#include <stdint.h>
#include "pinout.h"
#include <cstring>
#include <string>

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <driver/uart.h>



#define PROGMEM 
// typedef uint8_t     byte;
#define byte        uint8_t
typedef std::string String;

#define __FlashStringHelper     char

#define pgm_read_byte(addr)   (*(const unsigned char *)(addr))


#define F(x)    x

#define HIGH  (1)
#define LOW   (0)
#define SS  SPI_CS_PIN
#define LSBFIRST 0
#define MSBFIRST 1
#define SPI_LSBFIRST 0
#define SPI_MSBFIRST 1
#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3

//GPIO FUNCTIONS
#define INPUT             0x01
// Changed OUTPUT from 0x02 to behave the same as Arduino pinMode(pin,OUTPUT) 
// where you can read the state of pin even when it is set as OUTPUT
#define OUTPUT            0x03 
#define PULLUP            0x04
#define INPUT_PULLUP      0x05
#define PULLDOWN          0x08
#define INPUT_PULLDOWN    0x09
#define OPEN_DRAIN        0x10
#define OUTPUT_OPEN_DRAIN 0x13
#define ANALOG            0xC0




void digitalWrite(int pin, int level);
uint32_t millis();
void yield(void);
void pinMode(uint8_t pin, uint8_t mode);
int digitalRead(uint8_t pin);
void delayMicroseconds(uint32_t us);
void delay(uint32_t ms);

class SPISettings
{
public:
    SPISettings() :_clock(1000000), _bitOrder(SPI_MSBFIRST), _dataMode(SPI_MODE0) {}
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) :_clock(clock), _bitOrder(bitOrder), _dataMode(dataMode) {}
    uint32_t _clock;
    uint8_t  _bitOrder;
    uint8_t  _dataMode;
};

class SPIClass
{
public:
    SPIClass() {};
    ~SPIClass() {};
    void begin(gpio_num_t chipSelectPin, gpio_num_t reset, gpio_num_t mosi, gpio_num_t miso, gpio_num_t clk);
    void beginTransaction(SPISettings settings);
    void endTransaction(void);
    uint8_t transfer(uint8_t data);
private:
    spi_device_handle_t spi;
};


#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2


class Print
{
public:
    Print() {};
    virtual ~Print() {};
    // size_t printf(const char * format, ...)  __attribute__ ((format (printf, 2, 3)));
    // size_t print(const __FlashStringHelper *ifsh) { return print(reinterpret_cast<const char *>(ifsh)); }
    size_t print(const String &);
    size_t print(const char* s);
    size_t print(char);
    size_t print(unsigned char, int = DEC);
    size_t print(int, int = DEC);
    size_t print(unsigned int, int = DEC);
    size_t print(long, int = DEC);
    size_t print(unsigned long, int = DEC);
    size_t print(long long, int = DEC);
    size_t print(unsigned long long, int = DEC);
    size_t print(double, int = 2);
    size_t print(struct tm * timeinfo, const char * format = NULL);

    // size_t println(const __FlashStringHelper *ifsh) { return println(reinterpret_cast<const char *>(ifsh)); }
    size_t println(const String &s);
    size_t println(const char* s);
    size_t println(char);
    size_t println(unsigned char, int = DEC);
    size_t println(int, int = DEC);
    size_t println(unsigned int, int = DEC);
    size_t println(long, int = DEC);
    size_t println(unsigned long, int = DEC);
    size_t println(long long, int = DEC);
    size_t println(unsigned long long, int = DEC);
    size_t println(double, int = 2);
    size_t println(struct tm * timeinfo, const char * format = NULL);
    size_t println(void);
    
    // virtual void flush() { /* Empty implementation for backward compatibility */ }
    
};





class Stream: public Print
{
protected:
    unsigned long _timeout;      // number of milliseconds to wait for the next char before aborting timed read
    unsigned long _startMillis;  // used for timeout measurement
//     int timedRead();    // private method to read stream with timeout
//     int timedPeek();    // private method to peek stream with timeout
//     int peekNextDigit(); // returns the next numeric digit in the stream or -1 if timeout

public:
//     virtual int available() = 0;
//     virtual int read() = 0;
//     virtual int peek() = 0;

    Stream():_startMillis(0)
    {
        _timeout = 1000;
    }
    virtual ~Stream() {}

// // parsing methods

//     void setTimeout(unsigned long timeout);  // sets maximum milliseconds to wait for stream data, default is 1 second
//     unsigned long getTimeout(void);
      
//     bool find(const char *target);   // reads data from the stream until the target string is found
//     bool find(uint8_t *target)
//     {
//         return find((char *) target);
//     }
//     // returns true if target string is found, false if timed out (see setTimeout)

//     bool find(const char *target, size_t length);   // reads data from the stream until the target string of given length is found
//     bool find(const uint8_t *target, size_t length)
//     {
//         return find((char *) target, length);
//     }
//     // returns true if target string is found, false if timed out

//     bool find(char target)
//     {
//         return find (&target, 1);
//     }

//     bool findUntil(const char *target, const char *terminator);   // as find but search ends if the terminator string is found
//     bool findUntil(const uint8_t *target, const char *terminator)
//     {
//         return findUntil((char *) target, terminator);
//     }

//     bool findUntil(const char *target, size_t targetLen, const char *terminate, size_t termLen);   // as above but search ends if the terminate string is found
//     bool findUntil(const uint8_t *target, size_t targetLen, const char *terminate, size_t termLen)
//     {
//         return findUntil((char *) target, targetLen, terminate, termLen);
//     }

//     long parseInt(); // returns the first valid (long) integer value from the current position.
//     // initial characters that are not digits (or the minus sign) are skipped
//     // integer is terminated by the first character that is not a digit.

//     float parseFloat();               // float version of parseInt

//     virtual size_t readBytes(char *buffer, size_t length); // read chars from stream into buffer
//     virtual size_t readBytes(uint8_t *buffer, size_t length)
//     {
//         return readBytes((char *) buffer, length);
//     }
//     // terminates if length characters have been read or timeout (see setTimeout)
//     // returns the number of characters placed in the buffer (0 means no valid data found)

//     size_t readBytesUntil(char terminator, char *buffer, size_t length); // as readBytes with terminator character
//     size_t readBytesUntil(char terminator, uint8_t *buffer, size_t length)
//     {
//         return readBytesUntil(terminator, (char *) buffer, length);
//     }
//     // terminates if length characters have been read, timeout, or if the terminator character  detected
//     // returns the number of characters placed in the buffer (0 means no valid data found)

//     // Arduino String functions to be added here
//     virtual String readString();
//     String readStringUntil(char terminator);

// protected:
//     long parseInt(char skipChar); // as above but the given skipChar is ignored
//     // as above but the given skipChar is ignored
//     // this allows format characters (typically commas) in values to be ignored

//     float parseFloat(char skipChar);  // as above but the given skipChar is ignored
  
//     struct MultiTarget {
//       const char *str;  // string you're searching for
//       size_t len;       // length of string you're searching for
//       size_t index;     // index used by the search routine.
//     };

//   // This allows you to search for an arbitrary number of strings.
//   // Returns index of the target that is found first or -1 if timeout occurs.
//   int findMulti(struct MultiTarget *targets, int tCount);

};






class TwoWire: public Stream
{
public:
    TwoWire(uint8_t bus_num) {};
    ~TwoWire() {};
};



class HardwareSerial: public Stream
{
public:
    HardwareSerial(uart_port_t port);
    ~HardwareSerial() {};

    int available(void);
    int peek(void);
    int read(void);
    size_t read(uint8_t *buffer, size_t size);
    inline size_t read(char * buffer, size_t size)
    {
        return read((uint8_t*) buffer, size);
    }
    size_t write(uint8_t);
    size_t write(const uint8_t *buffer, size_t size);
    inline size_t write(const char * buffer, size_t size)
    {
        return write((uint8_t*) buffer, size);
    }
    inline size_t write(const char * s)
    {
        return write((uint8_t*) s, strlen(s));
    }
    inline size_t write(unsigned long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(unsigned int n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(int n)
    {
        return write((uint8_t) n);
    }

private:
    uart_port_t _port;
};








extern Print Serial;
extern SPIClass SPI;
extern HardwareSerial Serial2;


#endif // __ARDUINO_WRAPPER_HPP__