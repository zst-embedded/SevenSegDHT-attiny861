#ifndef MAIN_SEVENSEG_H
#define MAIN_SEVENSEG_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <math.h>

class TriSevenSeg {
public:
    enum DisplayMux {
        NONE = -1,
        A = 0,
        B = 1,
        C = 2,
    };
    static const uint8_t BLANK = 0xFF;
    static const uint8_t NUMBER[];
    static const uint8_t LETTER[];
    static const uint8_t getLetter(char c) {
        return LETTER[c - 'A'];
    }

private:
    enum DisplayMux mux = NONE;
    uint8_t brightness[3] = {0xFF, 0xFF, 0xFF};
    uint8_t display_font[3] = {0xFF, 0xFF, 0xFF};

public:
    void setBrightness(const TriSevenSeg::DisplayMux display, const uint8_t brightness);

    void setScreenDisplay(const TriSevenSeg::DisplayMux display, const uint8_t display_font);

    void setMux(const TriSevenSeg::DisplayMux display);

    void animateFadeIn(const TriSevenSeg::DisplayMux display, uint8_t brightness);
    void animateFadeOut(const TriSevenSeg::DisplayMux display);

    void animateSineIn(const TriSevenSeg::DisplayMux display, uint8_t brightness);
    void animateSineOut(const TriSevenSeg::DisplayMux display);

    inline void animateFadeOutAll() { animateFadeOut(TriSevenSeg::NONE); }
    inline void animateFadeInAll(uint8_t b) { animateFadeIn(TriSevenSeg::NONE, b); }

    inline void animateSineOutAll() { animateSineOut(TriSevenSeg::NONE); }
    inline void animateSineInAll(uint8_t b) { animateSineIn(TriSevenSeg::NONE, b); }

    TriSevenSeg::DisplayMux switchMux();
};

#endif //MAIN_SEVENSEG_H
