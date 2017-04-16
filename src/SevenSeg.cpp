#include "SevenSeg.h"

#define DualSevenSeg_FRAME_DELAY (12)
#define DualSevenSeg_FRAMES (25)
#define DualSevenSeg_FRAMES_INV (0.04)

const uint8_t DualSevenSeg::NUMBER[] = {
        0xc0, 0xf9, 0xa4, 0xb0, // 0, 1, 2, 3
        0x99, 0x92, 0x82, 0xf8, // 4, 5, 6, 7
        0x80, 0x90, // 8, 9
        0x88, 0x83, 0xc6, //   A B C
        0xa1, 0x86, 0x8e, // D E F
};
const uint8_t DualSevenSeg::LETTER[] = {
        0x88, 0x83, 0xc6, //   A B C
        0xa1, 0x86, 0x8e, 0xc2, // D E F G
        0x89, 0xcf, 0xe1, BLANK, // H I J _
        0xc7, 0xc8, 0xab, 0xc0, // L M N O
        0x8c, 0x98, 0xaf, 0x92, // P Q R S
        0x87, 0xc1, 0xe3, BLANK, // T U V _
        BLANK, 0x91, 0xa4, // _ Y Z
};

void DualSevenSeg::setBrightness(const DisplayMux display, const uint8_t brightness) {
    switch (display) {
        case NONE:
            this->brightness[1] = brightness;
        case LEFT:
            this->brightness[0] = brightness;
            break;
        case RIGHT:
            this->brightness[1] = brightness;
            break;
    }
}
void DualSevenSeg::animateSineOut(const DisplayMux display) {
    // cos x                    -> max 1, min -1 from 0 to pi
    // (cos x) + 1              -> max 2, min -1 from 0 to pi
    // 127 * ((cos x) + 1)      -> max 254, min 0 from 0 to pi
    // 127 * ((cos x * pi) + 1)      -> max 254, min 0 from 0 to 1
    // 127 * ((cos x * pi / 20) + 1)      -> max 254, min 0 from 0 to 20

    uint8_t origLeft = this->brightness[0];
    uint8_t origRight = this->brightness[1];
    uint8_t left, right;

    for (uint8_t step = 0; step < DualSevenSeg_FRAMES; step++) {
        left = (uint8_t) round( origLeft * 0.5 * (cos(step * M_PI * DualSevenSeg_FRAMES_INV) + 1) );
        right = (uint8_t) round( origRight * 0.5 * (cos(step * M_PI * DualSevenSeg_FRAMES_INV) + 1) );
        switch (display) {
            case NONE:
                this->brightness[1] = right;
            case LEFT:
                this->brightness[0] = left;
                break;
            case RIGHT:
                this->brightness[1] = right;
                break;
        }
        _delay_ms(DualSevenSeg_FRAME_DELAY);
    }
    setBrightness(display, 0);
    _delay_ms(300);
}

void DualSevenSeg::animateSineIn(const DisplayMux display, uint8_t brightness) {
    // cos x                    -> max 1, min -1 from 0 to pi
    // -cos x                   -> max -1, min +1 from 0 to pi

    for (uint8_t x = 0; x <= DualSevenSeg_FRAMES; x++) {
        uint8_t calculated = (uint8_t) round( brightness * 0.5 * (-cos(x * M_PI * DualSevenSeg_FRAMES_INV) + 1) );
        setBrightness(display, calculated);
        _delay_ms(DualSevenSeg_FRAME_DELAY);
    }
    setBrightness(display, brightness);

}

void DualSevenSeg::animateFadeOut(const DisplayMux display) {
    uint8_t stepLeft = this->brightness[0] * DualSevenSeg_FRAMES_INV;
    uint8_t stepRight = this->brightness[1] * DualSevenSeg_FRAMES_INV;

    for (uint8_t step = 0; step < DualSevenSeg_FRAMES; step++) {
        switch (display) {
            case NONE:
                this->brightness[1] -= stepRight;
            case LEFT:
                this->brightness[0] -= stepLeft;
                break;
            case RIGHT:
                this->brightness[1] -= stepRight;
                break;
        }
        _delay_ms(DualSevenSeg_FRAME_DELAY);
    }
    setBrightness(display, 0);
    _delay_ms(300);
}


void DualSevenSeg::animateFadeIn(const DisplayMux display, uint8_t brightness) {
    uint8_t step = brightness * DualSevenSeg_FRAMES_INV;

    for (uint8_t x = 0; x <= DualSevenSeg_FRAMES; x++) {
        setBrightness(display, step * x);
        _delay_ms(DualSevenSeg_FRAME_DELAY);
    }
    setBrightness(display, brightness);

}

void DualSevenSeg::setScreenDisplay(const DisplayMux display, const uint8_t display_font) {
    switch (display) {
        case LEFT:
            this->display_font[0] = display_font;
            break;
        case RIGHT:
            this->display_font[1] = display_font;
            break;
    }
}

void DualSevenSeg::setMux(const DisplayMux display) {
    this->mux = display;
    switch (display) {
        case LEFT:
            PORTA = (display_font[0] << 1) | (PORTA & 0x1);
            OCR1B = brightness[0];
            OCR1D = 0;
            break;
        case RIGHT:
            PORTA = (display_font[1] << 1) | (PORTA & 0x1);
            OCR1B = 0;
            OCR1D = brightness[1];
            break;
        case NONE:
            PORTA &= 0x1; // clear all from PA1-PA7
            OCR1B = 0;
            OCR1D = 0;
            break;
    }
}
void DualSevenSeg::switchMux() {
    switch (this->mux) {
        case LEFT:
            setMux(RIGHT);
            break;
        //case RIGHT:
        //case NONE:
        default:
            setMux(LEFT);
            break;
    }
}