/*
The MIT License (MIT)

Copyright (c) 2016 winlin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "SimpleDHT-AVR.h"

int SimpleDHT11::confirm(volatile uint8_t *PORT, volatile uint8_t *DDR, uint8_t bit, int us, uint8_t level) {
    // wait one more count to ensure.
    int cnt = us / 10 + 1;

    bool ok = false;
    for (int i = 0; i < cnt; i++) {
        //if (digitalRead(pin) != level) {
        *DDR &= ~(1<<bit);
        if ((*PORT & 1<<bit) != level) {
            ok = true;
            break;
        }
        //delayMicroseconds
        _delay_us(10);
    }

    if (!ok) {
        return -1;
    }
    return 0;
}

uint8_t SimpleDHT11::bits2byte(uint8_t data[8]) {
    uint8_t v = 0;
    for (int i = 0; i < 8; i++) {
        v += data[i] << (7 - i);
    }
    return v;
}

int SimpleDHT11::sample(volatile uint8_t *PORT, volatile uint8_t *DDR, uint8_t bit, uint8_t data[40]) {
    // empty output data.
    memset(data, 0, 40);

    // notify DHT11 to start:
    //    1. PULL LOW 20ms.
    //    2. PULL HIGH 20-40us.
    //    3. SET TO INPUT.
    //pinMode(pin, OUTPUT);
    *DDR |= 1<<bit;

    //digitalWrite(pin, LOW);
    *PORT &= ~(1<<bit);

    //delay(20);
    _delay_ms(20);

    //digitalWrite(pin, HIGH);
    *PORT |= 1<<bit;

    //delayMicroseconds(30);
    _delay_us(30);

    //pinMode(pin, INPUT);
    *DDR &= ~(1<<bit);


    // DHT11 starting:
    //    1. PULL LOW 80us
    //    2. PULL HIGH 80us
    if (confirm(PORT, DDR, bit, 80, LOW)) {
        return 100;
    }
    if (confirm(PORT, DDR, bit, 80, HIGH)) {
        return 101;
    }

    // DHT11 data transmite:
    //    1. 1bit start, PULL LOW 50us
    //    2. PULL HIGH 26-28us, bit(0)
    //    3. PULL HIGH 70us, bit(1)
    for (int j = 0; j < 40; j++) {
        if (confirm(PORT, DDR, bit, 50, LOW)) {
            return 102;
        }

        // read a bit, should never call method,
        // for the method call use more than 20us,
        // so it maybe failed to detect the bit0.
        bool ok = false;
        int tick = 0;
        for (int i = 0; i < 8; i++, tick++) {
            // if (digitalRead(pin) != HIGH) {
            *DDR &= ~(1<<bit);
            if ((*PORT & 1<<bit) != HIGH) {
                ok = true;
                break;
            }
            //delayMicroseconds(10);
            _delay_us(10);
        }
        if (!ok) {
            return 103;
        }
        data[j] = (tick > 3? 1:0);
    }

    // DHT11 EOF:
    //    1. PULL LOW 50us.
    if (confirm(PORT, DDR, bit, 50, LOW)) {
        return 104;
    }

    return 0;
}

int SimpleDHT11::parse(uint8_t data[40], uint8_t* ptemperature, uint8_t* phumidity) {
    uint8_t humidity = bits2byte(data);
    uint8_t humidity2 = bits2byte(data + 8);
    uint8_t temperature = bits2byte(data + 16);
    uint8_t temperature2 = bits2byte(data + 24);
    uint8_t check = bits2byte(data + 32);
    uint8_t expect = humidity + humidity2 + temperature + temperature2;
    if (check != expect) {
        return 105;
    }
    *ptemperature = temperature;
    *phumidity = humidity;
    return 0;
}

int SimpleDHT11::read(volatile uint8_t *PORT, volatile uint8_t *DDR, uint8_t bit, uint8_t* ptemperature, uint8_t* phumidity, uint8_t pdata[40]) {
    int ret = 0;

    uint8_t data[40] = {0};
    if ((ret = sample(PORT, DDR, bit, data)) != 0) {
        return ret;
    }

    uint8_t temperature = 0;
    uint8_t humidity = 0;
    if ((ret = parse(data, &temperature, &humidity)) != 0) {
        return ret;
    }

    if (pdata) {
        memcpy(pdata, data, 40);
    }
    if (ptemperature) {
        *ptemperature = temperature;
    }
    if (phumidity) {
        *phumidity = humidity;
    }

    return ret;
}