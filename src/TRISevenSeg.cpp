#include "TriSevenSeg.h"

#define TriSevenSeg_FRAME_DELAY (12)
#define TriSevenSeg_FRAMES (25)
#define TriSevenSeg_FRAMES_INV (0.04)

const uint8_t TriSevenSeg::NUMBER[] = {
        0xc0, 0xf9, 0xa4, 0xb0, // 0, 1, 2, 3
        0x99, 0x92, 0x82, 0xf8, // 4, 5, 6, 7
        0x80, 0x90, // 8, 9
        0x88, 0x83, 0xc6, //   A B C
        0xa1, 0x86, 0x8e, // D E F
};
const uint8_t TriSevenSeg::LETTER[] = {
        0x88, 0x83, 0xc6, //   A B C
        0xa1, 0x86, 0x8e, 0xc2, // D E F G
        0x89, 0xcf, 0xe1, BLANK, // H I J _
        0xc7, 0xc8, 0xab, 0xc0, // L M N O
        0x8c, 0x98, 0xaf, 0x92, // P Q R S
        0x87, 0xc1, 0xe3, BLANK, // T U V _
        BLANK, 0x91, 0xa4 // _ Y Z
};

void TriSevenSeg::setBrightness(const TriSevenSeg::DisplayMux display, const uint8_t brightness) {
    if (display == NONE) {
        this->brightness[A] = brightness;
        this->brightness[B] = brightness;
        this->brightness[C] = brightness;
    } else {
        this->brightness[display] = brightness;
    }
}
void TriSevenSeg::animateSineOut(const DisplayMux display) {
    // cos x                    -> max 1, min -1 from 0 to pi
    // (cos x) + 1              -> max 2, min -1 from 0 to pi
    // 127 * ((cos x) + 1)      -> max 254, min 0 from 0 to pi
    // 127 * ((cos x * pi) + 1)      -> max 254, min 0 from 0 to 1
    // 127 * ((cos x * pi / 20) + 1)      -> max 254, min 0 from 0 to 20

    const uint8_t orig[] = {
            this->brightness[0],
            this->brightness[1],
            this->brightness[2]
    };
    uint8_t calculated[3];

    for (uint8_t step = 0; step < TriSevenSeg_FRAMES; step++) {
        for (uint8_t arr = 0; arr < 3; arr++) {
            calculated[arr] = (uint8_t) round( orig[arr] * 0.5 * (cos(step * M_PI * TriSevenSeg_FRAMES_INV) + 1) );
            if (display == NONE) {
                this->brightness[arr] = calculated[arr];
                // Apply calculated to all 3
            }
        }
        if (display != NONE) {
            this->brightness[display] = calculated[display];
        }
        _delay_ms(TriSevenSeg_FRAME_DELAY);
    }
    setBrightness(display, 0);
    _delay_ms(300);
}

void TriSevenSeg::animateSineIn(const DisplayMux display, uint8_t brightness) {
    // cos x                    -> max 1, min -1 from 0 to pi
    // -cos x                   -> max -1, min +1 from 0 to pi

    for (uint8_t x = 0; x <= TriSevenSeg_FRAMES; x++) {
        uint8_t calculated = (uint8_t) round( brightness * 0.5 * (-cos(x * M_PI * TriSevenSeg_FRAMES_INV) + 1) );
        setBrightness(display, calculated);
        _delay_ms(TriSevenSeg_FRAME_DELAY);
    }
    setBrightness(display, brightness);

}

void TriSevenSeg::animateFadeOut(const DisplayMux display) {
    uint8_t steps[] = {
            this->brightness[0] * TriSevenSeg_FRAMES_INV,
            this->brightness[1] * TriSevenSeg_FRAMES_INV,
            this->brightness[2] * TriSevenSeg_FRAMES_INV
    };

    for (uint8_t step = 0; step < TriSevenSeg_FRAMES; step++) {
        if (display == NONE) {
            this->brightness[0] -= steps[0];
            this->brightness[1] -= steps[1];
            this->brightness[2] -= steps[2];
        } else {
            this->brightness[display] -= steps[display];
        }
        _delay_ms(TriSevenSeg_FRAME_DELAY);
    }
    setBrightness(display, 0);
    _delay_ms(300);
}


void TriSevenSeg::animateFadeIn(const DisplayMux display, uint8_t brightness) {
    uint8_t step = brightness * TriSevenSeg_FRAMES_INV;

    for (uint8_t x = 0; x <= TriSevenSeg_FRAMES; x++) {
        setBrightness(display, step * x);
        _delay_ms(TriSevenSeg_FRAME_DELAY);
    }
    setBrightness(display, brightness);

}

void TriSevenSeg::setScreenDisplay(const DisplayMux display, const uint8_t display_font) {
    this->display_font[display] = display_font;
}

void TriSevenSeg::setMux(const DisplayMux display) {
    this->mux = display;
    switch (display) {
        case A:
            PORTA = (~(display_font[0] << 1)) | (PORTA & 0x1);
            // Display A is common cathode
            OCR1A = brightness[0];
            OCR1B = 0;
            OCR1D = 0;
            break;
        case B:
            PORTA = (display_font[1] << 1) | (PORTA & 0x1);
            OCR1A = 0;
            OCR1B = brightness[1];
            OCR1D = 0;
            break;
        case C:
            PORTA = (display_font[2] << 1) | (PORTA & 0x1);
            OCR1A = 0;
            OCR1B = 0;
            OCR1D = brightness[2];
            break;
        case NONE:
            PORTA &= 0x1; // clear all from PA1-PA7
            OCR1A = 0;
            OCR1B = 0;
            OCR1D = 0;
            break;
    }
}
TriSevenSeg::DisplayMux TriSevenSeg::switchMux() {
    switch (this->mux) {
        case A:
            setMux(B);
            break;
        case B:
            setMux(C);
            break;
        default:
            setMux(A);
            break;
    }
    return this->mux;
}