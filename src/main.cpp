/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "TRISevenSeg.h"
#include "SimpleDHT.h"

/* Multiplexing rate of seven segments on Timer 0 */
#define SEVSEG_REFRESH (40) // 1/(8M/1024)*40 = 5.12 msec

TriSevenSeg sevenSeg;
volatile bool buttonPressed = false;
volatile uint8_t mode = 0;

void displayHumiTemp(uint8_t mode, uint8_t humidity, uint8_t temperature);
void displayFunction(uint8_t humidity, uint8_t temperature);

bool hasDoneSplashScreen = false;

int main(void) {
    // Setup IO
    DDRA = 0xFE;
    DDRB |= _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(5);
    PORTB |= _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(5);
    PORTB |= _BV(6); // internal pullup

    // Setup INT0 on pin 9
    MCUCR |= 1<<ISC01 | 1<<ISC00; // rising edge of INT0 or INT1 generates an interrupt request.
    GIMSK |= 1<<INT0; // enable INT0 interrupt

    // Setup OCR1A -> Red 7-seg PWM
    TCCR1A |= 3 << COM1A0 // Set on compare match (because common cathode)
            | 1 << PWM1A; // Enable PWM on OCR1B

    // Setup OCR1B -> Left 7-seg PWM
    TCCR1A |= 1 << COM1B1 // Clear on compare match
              | 1 << PWM1B; // Enable PWM on OCR1B
    TCCR1B |= 1 << CS10; // Pre-scaler of 1

    // Setup OCR1D -> Right 7-seg PWM
    TCCR1C |= 1 << COM1D1 // Clear on compare match
              | 1 << PWM1D; // Enable PWM on OCR1D

    // Setup Timer0 CTC -> Multiplex 7-seg
    TIMSK |= 1 << OCIE0A; // Enable Output Compare Match interrupt
    TCCR0A |= 1 << CTC0; // CTC mode
    TCCR0B |= 1 << CS02 | 1 << CS00; // Pre-scaler of 1024
    OCR0A = SEVSEG_REFRESH;

    sei();
    /*The OCF0A bit is set when a Compare Match occurs between the Timer/Counter0 and the data
    in OCR0A – Output Compare Register0. OCF0A is cleared by hardware when executing the corresponding
    interrupt handling vector. Alternatively, OCF0A is cleared by writing a logic one to
    the flag. When the I-bit in SREG, OCIE0A (Timer/Counter0 Compare Match Interrupt Enable),
    and OCF0A are set, the Timer/Counter0 Compare Match Interrupt is executed.
     */
    _delay_ms(1000);
    sevenSeg.setScreenDisplay(TriSevenSeg::A, ~(1<<6)); // Dash symbol
    sevenSeg.setScreenDisplay(TriSevenSeg::B, TriSevenSeg::getLetter('H'));
    sevenSeg.setScreenDisplay(TriSevenSeg::C, TriSevenSeg::getLetter('I'));
    sevenSeg.animateSineInAll(255);

    SimpleDHT11 dht11;
    const SimpleDHT11::pinType pin_type = {
            .ddr = &DDRB, .pin = &PINB, .port = &PORTB, .pos = PB4
    };
    uint8_t temperature = 0;
    uint8_t humidity = 0;
    buttonPressed = false;
    while (1) {
        if ((mode == 0 || mode == 1) && (dht11.read(pin_type, &temperature, &humidity, NULL) || humidity <= 0)) {
            sevenSeg.animateSineOutAll();
            sevenSeg.setScreenDisplay(TriSevenSeg::A, ~(1<<6)); // Dash symbol
            sevenSeg.setScreenDisplay(TriSevenSeg::B, ~(1<<6)); // Dash symbol
            sevenSeg.setScreenDisplay(TriSevenSeg::C, ~(1<<6)); // Dash symbol
            sevenSeg.animateSineInAll(100);
            //Serial.print("Read DHT11 failed.");
        } else {
            //displayHumiTemp(mode, humidity, temperature);
            displayFunction(humidity, temperature);
            //Serial.print((int)temperature); Serial.print(" *C, ");
            //Serial.print((int)humidity); Serial.println(" %");
            if (buttonPressed) {
                _delay_ms(250);
                buttonPressed = false;
                mode++;
                if (mode > 3) {
                    mode = 0;
                }
                hasDoneSplashScreen = false;
            }
        }
    }
}



#define DELAY_SHORT (40)
#define DELAY_LONG (100)
#define DELAY_LONGER (220)

void displayLetters(TriSevenSeg * sevenSeg, char leftch, char rightch) {
    sevenSeg->setScreenDisplay(TriSevenSeg::B, TriSevenSeg::getLetter(leftch));
    sevenSeg->setScreenDisplay(TriSevenSeg::C, TriSevenSeg::getLetter(rightch));
}


bool displayNumber(TriSevenSeg * sevenSeg, uint8_t number) {
    sevenSeg->setScreenDisplay(TriSevenSeg::B, TriSevenSeg::NUMBER[(uint8_t) (number / 10)]);
    sevenSeg->setScreenDisplay(TriSevenSeg::C, TriSevenSeg::NUMBER[(uint8_t) (number % 10)]);
}


bool displayNumberADCPercent(TriSevenSeg * sevenSeg, volatile uint16_t * number, uint8_t delay) {
    uint16_t oldNumber = *number;
    //ADC *= 0.09775; // result/1024*100

    sevenSeg->animateSineOutAll();
    sevenSeg->setScreenDisplay(TriSevenSeg::A, TriSevenSeg::getLetter('P'));
    sevenSeg->setScreenDisplay(TriSevenSeg::B, TriSevenSeg::NUMBER[(uint8_t) (*number * 0.09775) / 10]);
    sevenSeg->setScreenDisplay(TriSevenSeg::C, TriSevenSeg::NUMBER[(uint8_t) (*number * 0.09775) % 10]);
    if (buttonPressed) return false;
    sevenSeg->animateSineInAll(255);

    for (uint8_t delayLeft = delay;
         delayLeft > 0 && !buttonPressed;
         delayLeft--) {
        if (oldNumber != *number) {
            sevenSeg->setScreenDisplay(TriSevenSeg::A, TriSevenSeg::getLetter('P'));
            sevenSeg->setScreenDisplay(TriSevenSeg::B, TriSevenSeg::NUMBER[(uint8_t) (*number * 0.09775) / 10]);
            sevenSeg->setScreenDisplay(TriSevenSeg::C, TriSevenSeg::NUMBER[(uint8_t) (*number * 0.09775) % 10]);
        }
        _delay_ms(10);
    }
    //return buttonPressed ? false : true;
    return !buttonPressed;
}

// returns true on button press
bool delayWithButtonCheck(uint8_t delay) {
    if (buttonPressed) return true;
    for (uint8_t delayLeft = delay;
         delayLeft > 0 && !buttonPressed;
         delayLeft--) {
        _delay_ms(10);
    }
    //return buttonPressed ? false : true;
    return buttonPressed;
}
void displayHumiTemp(uint8_t mode, uint8_t humidity, uint8_t temperature) {
    switch (mode) {
        case 0: // hum and temp
        case 1: // hum only
            sevenSeg.animateSineOutAll();
            sevenSeg.setScreenDisplay(TriSevenSeg::A, ~(1<<6)); // Dash symbol
            displayLetters(&sevenSeg, 'H', 'V');
            sevenSeg.animateSineInAll(255);
            if (delayWithButtonCheck(DELAY_SHORT)) return;

            sevenSeg.animateSineOutAll();
            sevenSeg.setScreenDisplay(TriSevenSeg::A, TriSevenSeg::getLetter('H'));
            displayNumber(&sevenSeg, humidity);
            sevenSeg.animateSineInAll(255);
            if (delayWithButtonCheck(DELAY_LONG)) return;

            if (mode == 1) return;
            // else mode 0 continue to temp
        case 2: // temp
            sevenSeg.animateSineOutAll();
            displayLetters(&sevenSeg, 'T', 'N');
            sevenSeg.animateSineInAll(255);
            delayWithButtonCheck(DELAY_SHORT);

            sevenSeg.animateSineOutAll();
            displayNumber(&sevenSeg, temperature);
            sevenSeg.animateSineInAll(255);
            if (delayWithButtonCheck(DELAY_LONG)) return;
            return;
        case 3: // ADC0
        case 4: // internal temp
        { // http://stackoverflow.com/a/23599822
            ADCSRA |= 1 << ADEN | 1 << ADPS1 | 1 << ADPS2; // Enable ADC with Prescaler of 64
            ADCSRA |= 1<<ADATE; // Auto trigger
            //ADCSRB &= ~(1<<ADTS0 | 1<<ADTS1 | 1<<ADTS2); // Free running auto trigger
            ADCSRB |= (1<<ADTS0 | 1<<ADTS1); // Timer 0A CTC
            // 0 < REFS0  Vcc as voltage ref and 0 < ADLAR  right adjust result

            if (mode == 4) {
                // ADC11 -> int temp sensor
                ADMUX = 0b11111; // MUX4:0
                ADCSRB |= 1 << MUX5; // set MUX5
            } else {
                ADMUX = 0b0; // ADC0 -> PA0
                ADCSRB &= ~(1 << MUX5); // clear MUX5
                DIDR0 |= 1<<ADC0D;
            }
            ADCSRA |= 1 << ADSC; // start ADC conversion
            while (ADCSRA & (1 << ADSC)); // wait until conversion done

            if (mode == 4) {
                uint16_t result = ADC; // read high bit after the low
                uint16_t int_temp = result - 35; // minus away calibration
                sevenSeg.animateSineOutAll();
                displayLetters(&sevenSeg, 'T', 'D');
                sevenSeg.animateSineInAll(255);
                delayWithButtonCheck(DELAY_SHORT);

                sevenSeg.animateSineOutAll();
                displayNumber(&sevenSeg, int_temp);
                sevenSeg.animateSineInAll(255);
                if (delayWithButtonCheck(DELAY_LONG)) return;
            } else {
                sevenSeg.animateSineOutAll();
                displayLetters(&sevenSeg, 'P', 'T');
                sevenSeg.animateSineInAll(255);
                delayWithButtonCheck(20);

                if (!displayNumberADCPercent(&sevenSeg, &ADC, DELAY_LONG * 2)) return;
            }

            DIDR0 &= ~(1<<ADC0D);
            ADCSRA &= ~(1<ADEN | 1<<ADATE); // disable ADC
        }
            return;
        default:
            displayHumiTemp(0, humidity, temperature);
            return;
    }
}
void displayFunction(uint8_t humidity, uint8_t temperature) {
    switch (mode) {
        case 0: // hum only
            if (!hasDoneSplashScreen) {
                hasDoneSplashScreen = true;
                sevenSeg.animateSineOutAll();
                sevenSeg.setScreenDisplay(TriSevenSeg::A, ~(1<<6)); // Dash symbol
                displayLetters(&sevenSeg, 'H', 'V');
                sevenSeg.animateSineInAll(255);
                if (delayWithButtonCheck(DELAY_SHORT)) return;

                sevenSeg.animateSineOutAll();
                sevenSeg.setScreenDisplay(TriSevenSeg::A, TriSevenSeg::getLetter('H'));
                displayNumber(&sevenSeg, humidity);
                sevenSeg.animateSineInAll(255);
            } else {
                sevenSeg.setScreenDisplay(TriSevenSeg::A, TriSevenSeg::getLetter('H'));
                displayNumber(&sevenSeg, humidity);
                if (delayWithButtonCheck(DELAY_LONGER)) return;
            }
            return;
        case 1: // temp only
            if (!hasDoneSplashScreen) {
                hasDoneSplashScreen = true;
                sevenSeg.animateSineOutAll();
                sevenSeg.setScreenDisplay(TriSevenSeg::A, ~(1<<6)); // Dash symbol
                displayLetters(&sevenSeg, 'T', 'N');
                sevenSeg.animateSineInAll(255);
                if (delayWithButtonCheck(DELAY_SHORT)) return;

                sevenSeg.animateSineOutAll();
                sevenSeg.setScreenDisplay(TriSevenSeg::A, TriSevenSeg::getLetter('T'));
                displayNumber(&sevenSeg, temperature);
                sevenSeg.animateSineInAll(255);
            } else {
                sevenSeg.setScreenDisplay(TriSevenSeg::A, TriSevenSeg::getLetter('T'));
                displayNumber(&sevenSeg, temperature);
                if (delayWithButtonCheck(DELAY_LONGER)) return;
            }
            return;
        case 2: // ADC0
        case 3: // internal temp
        { // http://stackoverflow.com/a/23599822
            ADCSRA |= 1 << ADEN | 1 << ADPS1 | 1 << ADPS2; // Enable ADC with Prescaler of 64
            ADCSRA |= 1<<ADATE; // Auto trigger
            //ADCSRB &= ~(1<<ADTS0 | 1<<ADTS1 | 1<<ADTS2); // Free running auto trigger
            ADCSRB |= (1<<ADTS0 | 1<<ADTS1); // Timer 0A CTC
            // 0 < REFS0  Vcc as voltage ref and 0 < ADLAR  right adjust result

            if (mode == 3) {
                // ADC11 -> int temp sensor
                ADMUX = 0b11111; // MUX4:0
                ADCSRB |= 1 << MUX5; // set MUX5
            } else { // ADC0 pot
                ADMUX = 0b0; // ADC0 -> PA0
                ADCSRB &= ~(1 << MUX5); // clear MUX5
                DIDR0 |= 1<<ADC0D;
            }
            ADCSRA |= 1 << ADSC; // start ADC conversion
            while (ADCSRA & (1 << ADSC)); // wait until conversion done

            if (mode == 3) {
                uint16_t result = ADC; // read high bit after the low
                uint16_t int_temp = result - 35; // minus away calibration
                if (!hasDoneSplashScreen) {
                    hasDoneSplashScreen = true;
                    sevenSeg.animateSineOutAll();
                    sevenSeg.setScreenDisplay(TriSevenSeg::A, ~(1<<6)); // Dash symbol
                    displayLetters(&sevenSeg, 'T', 'D');
                    sevenSeg.animateSineInAll(255);
                    if (delayWithButtonCheck(DELAY_SHORT)) return;
                    sevenSeg.animateSineOutAll();
                    sevenSeg.setScreenDisplay(TriSevenSeg::A, TriSevenSeg::getLetter('D'));
                    displayNumber(&sevenSeg, int_temp);
                    sevenSeg.animateSineInAll(255);
                    if (delayWithButtonCheck(DELAY_SHORT)) return;
                } else {
                    sevenSeg.setScreenDisplay(TriSevenSeg::A, TriSevenSeg::getLetter('D'));
                    displayNumber(&sevenSeg, int_temp);
                    if (delayWithButtonCheck(DELAY_SHORT)) return;
                }
            } else {
                //ADC *= 0.09775; // result/1024*100
                if (!hasDoneSplashScreen) {
                    hasDoneSplashScreen = true;
                    sevenSeg.animateSineOutAll();
                    sevenSeg.setScreenDisplay(TriSevenSeg::A, ~(1<<6)); // Dash symbol
                    displayLetters(&sevenSeg, 'P', 'T');
                    sevenSeg.animateSineInAll(255);

                    if (delayWithButtonCheck(20)) return;
                    sevenSeg.animateSineOutAll();
                    sevenSeg.setScreenDisplay(TriSevenSeg::A, TriSevenSeg::getLetter('P'));
                    sevenSeg.setScreenDisplay(TriSevenSeg::B, TriSevenSeg::NUMBER[(uint8_t) (ADC * 0.09775) / 10]);
                    sevenSeg.setScreenDisplay(TriSevenSeg::C, TriSevenSeg::NUMBER[(uint8_t) (ADC * 0.09775) % 10]);
                    sevenSeg.animateSineInAll(255);
                } else {
                    if (buttonPressed) return;
                    sevenSeg.setScreenDisplay(TriSevenSeg::A, TriSevenSeg::getLetter('P'));
                    sevenSeg.setScreenDisplay(TriSevenSeg::B, TriSevenSeg::NUMBER[(uint8_t) (ADC * 0.09775) / 10]);
                    sevenSeg.setScreenDisplay(TriSevenSeg::C, TriSevenSeg::NUMBER[(uint8_t) (ADC * 0.09775) % 10]);
                    if (delayWithButtonCheck(10)) return;
                    return;
                }
            }

            DIDR0 &= ~(1<<ADC0D);
            ADCSRA &= ~(1<ADEN | 1<<ADATE); // disable ADC
        }
            return;
        default:
            mode = 0;
            return;
    }
}

ISR(TIMER0_COMPA_vect) {
    TriSevenSeg::DisplayMux final = sevenSeg.switchMux();
    if (final == 0) {
        if (!buttonPressed) {
            PORTB &= ~_BV(2);
        } else {
            PORTB |= _BV(2);
        }
    }
}

ISR(INT0_vect) {
    buttonPressed = true;
}




// DHT11 sampling rate is 1HZ.
/*
int chk = dht.read11(&PORTB, &DDRB, PB4);
if (chk == DHTLIB_OK) {
    sevenSeg.animateFadeOutAll();
    sevenSeg.setScreenDisplay(DualSevenSeg::LEFT, DualSevenSeg::getLetter('H'));
    sevenSeg.setScreenDisplay(DualSevenSeg::RIGHT, DualSevenSeg::getLetter('V'));
    sevenSeg.animateFadeInAll(255);
    _delay_ms(500);
    sevenSeg.animateFadeOutAll();
    sevenSeg.setScreenDisplay(DualSevenSeg::LEFT, DualSevenSeg::NUMBER[ (uint8_t) (dht.humidity / 10)]);
    sevenSeg.setScreenDisplay(DualSevenSeg::LEFT, DualSevenSeg::NUMBER[ (uint8_t) ((int8_t) dht.humidity % 10)]);
    sevenSeg.animateFadeInAll(255);
    _delay_ms(1000);

    sevenSeg.animateFadeOutAll();
    sevenSeg.setScreenDisplay(DualSevenSeg::LEFT, DualSevenSeg::getLetter('T'));
    sevenSeg.setScreenDisplay(DualSevenSeg::RIGHT, DualSevenSeg::getLetter('N'));
    _delay_ms(100);

} else {
    sevenSeg.animateFadeOutAll();
    switch (chk) {
        case DHTLIB_ERROR_CHECKSUM:
            //Serial.print("Checksum error,\t");
            sevenSeg.setScreenDisplay(DualSevenSeg::LEFT, DualSevenSeg::getLetter('E'));
            sevenSeg.setScreenDisplay(DualSevenSeg::RIGHT, DualSevenSeg::getLetter('C'));
            break;
        case DHTLIB_ERROR_TIMEOUT:
            //Serial.print("Time out error,\t");
            sevenSeg.setScreenDisplay(DualSevenSeg::LEFT, DualSevenSeg::getLetter('E'));
            sevenSeg.setScreenDisplay(DualSevenSeg::RIGHT, DualSevenSeg::getLetter('T'));
            break;
        case DHTLIB_ERROR_CONNECT:
            //Serial.print("Connect error,\t");
            sevenSeg.setScreenDisplay(DualSevenSeg::LEFT, DualSevenSeg::getLetter('C'));
            sevenSeg.setScreenDisplay(DualSevenSeg::RIGHT, DualSevenSeg::getLetter('N'));
            break;
        case DHTLIB_ERROR_ACK_L:
            //Serial.print("Ack Low error,\t");
            sevenSeg.setScreenDisplay(DualSevenSeg::LEFT, DualSevenSeg::getLetter('A'));
            sevenSeg.setScreenDisplay(DualSevenSeg::RIGHT, DualSevenSeg::getLetter('L'));
            break;
        case DHTLIB_ERROR_ACK_H:
            //Serial.print("Ack High error,\t");
            sevenSeg.setScreenDisplay(DualSevenSeg::LEFT, DualSevenSeg::getLetter('A'));
            sevenSeg.setScreenDisplay(DualSevenSeg::RIGHT, DualSevenSeg::getLetter('H'));
            break;
        default:
            sevenSeg.setScreenDisplay(DualSevenSeg::LEFT, DualSevenSeg::getLetter('U'));
            sevenSeg.setScreenDisplay(DualSevenSeg::RIGHT, DualSevenSeg::getLetter('N'));
            //Serial.print("Unknown error,\t");
            break;
    }

    sevenSeg.animateFadeInAll(255);
}
_delay_ms(2000);
 */


// DISPLAY DATA
// dht.humidity
// dht.temperature


/*
for (uint8_t bit = 0; bit < 8; bit++) {
    sevenSeg.animateFadeOut(DualSevenSeg::LEFT);
    sevenSeg.setScreenDisplay(DualSevenSeg::LEFT, DualSevenSeg::NUMBER[bit + 1]);
    sevenSeg.animateFadeIn(DualSevenSeg::LEFT, 255);

    //sevenSeg.setBrightness(DualSevenSeg::LEFT, 32*bit);
    holder ^= _BV(bit);
    sevenSeg.setScreenDisplay(DualSevenSeg::RIGHT, holder);
    _delay_ms(100);
    //PORTB ^= _BV(4);
    //OCR1B+=5;
    //OCR1D+=5;
}
 */