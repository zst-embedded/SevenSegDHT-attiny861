//
// Modified by Manzel for AVR.
//
//    FILE: dht.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.20
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
//     URL: http://arduino.cc/playground/Main/DHTLib
//
// HISTORY:
// see dht.cpp file
//


#ifndef MAIN_DHT_AVR_H
#define MAIN_DHT_AVR_H

#include <util/delay.h>
#include <stdint.h>

#define DHT_LIB_VERSION "0.1.20"

#define DHTLIB_OK                   0
#define DHTLIB_ERROR_CHECKSUM       -1
#define DHTLIB_ERROR_TIMEOUT        -2
#define DHTLIB_ERROR_CONNECT        -3
#define DHTLIB_ERROR_ACK_L          -4
#define DHTLIB_ERROR_ACK_H          -5

#define DHTLIB_DHT11_WAKEUP         18
#define DHTLIB_DHT_WAKEUP           1

#define DHTLIB_DHT11_LEADING_ZEROS  1
#define DHTLIB_DHT_LEADING_ZEROS    6

// max timeout is 100 usec.
// For a 16 Mhz proc 100 usec is 1600 clock cycles
// loops using DHTLIB_TIMEOUT use at least 4 clock cycli
// so 100 us takes max 400 loops
// so by dividing F_CPU by 40000 we "fail" as fast as possible
#define DHTLIB_TIMEOUT (F_CPU * 1e-4 * 0.25) //400

class DHT {
public:
    // return values:
    // DHTLIB_OK
    // DHTLIB_ERROR_CHECKSUM
    // DHTLIB_ERROR_TIMEOUT
    // DHTLIB_ERROR_CONNECT
    // DHTLIB_ERROR_ACK_L
    // DHTLIB_ERROR_ACK_H
    int8_t read11(volatile uint8_t *PORT, volatile uint8_t *DDR, uint8_t bit);

    int8_t read(volatile uint8_t *PORT, volatile uint8_t *DDR, uint8_t bit);

    inline int8_t read21(volatile uint8_t *PORT, volatile uint8_t *DDR, uint8_t bit) {
        return read(PORT, DDR, bit);
    };

    inline int8_t read22(volatile uint8_t *PORT, volatile uint8_t *DDR, uint8_t bit) {
        return read(PORT, DDR, bit);
    };

    inline int8_t read33(volatile uint8_t *PORT, volatile uint8_t *DDR, uint8_t bit) {
        return read(PORT, DDR, bit);
    };

    inline int8_t read44(volatile uint8_t *PORT, volatile uint8_t *DDR, uint8_t bit) {
        return read(PORT, DDR, bit);
    };


    double humidity;
    double temperature;

private:
    static const uint8_t LOW = 0;
    static const uint8_t HIGH = 0;
    uint8_t bits[5];  // buffer to receive data
    int8_t _readSensor(volatile uint8_t *PORT, volatile uint8_t *DDR, uint8_t bit, uint8_t wakeupDelay, uint8_t leadingZeroBits);

    inline uint16_t word(uint8_t high, uint8_t low) {
        return (high << 8) | low;
    }
    inline uint16_t min_u16(uint16_t a, uint16_t b) {
        return (a > b) ? b : a;
    }

};

#endif
