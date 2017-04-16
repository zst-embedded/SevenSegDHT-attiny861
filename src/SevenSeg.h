#ifndef MAIN_SEVENSEG_H
#define MAIN_SEVENSEG_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <math.h>

class DualSevenSeg {
public:
    enum DisplayMux {
        NONE = -1,
        LEFT = 0,
        RIGHT = 1,
    };
    static const uint8_t BLANK = 0xFF;
    static const uint8_t NUMBER[];
    static const uint8_t LETTER[];
    static const uint8_t getLetter(char c) {
        return LETTER[c - 'A'];
    }

private:
    enum DisplayMux mux = NONE;
    uint8_t brightness[2] = {0xFF, 0xFF};
    uint8_t display_font[2] = {0xFF, 0xFF};

public:
    void setBrightness(const DisplayMux display, const uint8_t brightness);

    void setScreenDisplay(const DisplayMux display, const uint8_t display_font);

    void setMux(const DisplayMux display);

    void animateFadeIn(const DisplayMux display, uint8_t brightness);
    void animateFadeOut(const DisplayMux display);

    void animateSineIn(const DisplayMux display, uint8_t brightness);
    void animateSineOut(const DisplayMux display);

    inline void animateFadeOutAll() { animateFadeOut(DualSevenSeg::NONE); }
    inline void animateFadeInAll(uint8_t b) { animateFadeIn(DualSevenSeg::NONE, b); }

    inline void animateSineOutAll() { animateSineOut(DualSevenSeg::NONE); }
    inline void animateSineInAll(uint8_t b) { animateSineIn(DualSevenSeg::NONE, b); }


    void switchMux();
};

#endif //MAIN_SEVENSEG_H
