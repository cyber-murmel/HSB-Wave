#include <avr/io.h>             // this contains all the IO port definitions
#include <avr/interrupt.h>      // definitions for interrupts
#include <avr/sleep.h>          // definitions for power-down modes
#include <avr/pgmspace.h>       // definitions or keeping constants in program memory
#include <stdint.h>
#include "color.h"
#include "pinman.h"

#define ITE_MIN (1)
#define ITE_STEP (1)
#define ITE_MAX (10)
#define HUE_STEP (4)
#define SAT (100)
#define VAL (50)

#ifndef COL_REG
#define COL_REG B
#endif

#ifndef RED_PIN
#define RED_PIN (1)
#endif
#ifndef GRN_PIN
#define GRN_PIN (2)
#endif
#ifndef BLU_PIN
#define BLU_PIN (4)
#endif

#define RED_ON  PORT(COL_REG) &= ~(1<<RED_PIN);
#define RED_OFF PORT(COL_REG) |= 1<<RED_PIN;
#define GRN_ON  PORT(COL_REG) &= ~(1<<GRN_PIN);
#define GRN_OFF PORT(COL_REG) |= 1<<GRN_PIN;
#define BLU_ON  PORT(COL_REG) &= ~(1<<BLU_PIN);
#define BLU_OFF PORT(COL_REG) |= 1<<BLU_PIN;


uint16_t hue = 0;
uint32_t ite = ITE_MIN;
color_t color;

ISR(PCINT0_vect) {
  ite = ITE_MIN;
  hue = 0;
}

// This function delays the specified number of 10 microseconds
void delay_ten_us(unsigned long int us) {
  unsigned long int count;
  const unsigned long int DelayCount=6;  // this value was determined by trial and error

  while (us != 0) {
    // Toggling PB5 is done here to force the compiler to do this loop, rather than optimize it away
    //   NOTE:  Below you will see: "0b00100000".
    //   This is an 8-bit binary number with all bits set to 0 except for the the 6th one from the right.
    //   Since bits in binary numbers are labeled starting from 0, the bit in this binary number that is set to 1
    //     is called PB5, which is the one unsed PORTB pin, which is why we can use it here
    //     (to fool the optimizing compiler to not optimize away this delay loop).
    for (count=0; count <= DelayCount; count++) {PINB |= 0b00100000;};
    us--;
  }
}

void pulseIR(void) {
  // We will use Timer 0 to pulse the IR LED at 38KHz
  //
  // We pulse the IR emitter for only 170 microseconds.  We do this to save battery power.
  //   The minimum number of 38KHz pulses that the IR detector needs to detect IR is 6.
  //   170 microseconds gives slightly more than 6 periods of 38KHz, which is enough to
  //   reflect off of your hand and hit the IR detector.
  //
  // start up Timer0 in CTC Mode at about 38KHz to drive the IR emitter on output OC0A:
  //   8-bit Timer0 OC0A (PB0, pin 5) is set up for CTC mode, toggling output on each compare
  //   Fclk = Clock = 8MHz
  //   Prescale = 1
  //   OCR0A = 104
  //   F = Fclk / (2 * Prescale * (1 + OCR0A) ) = 38KHz
  //
  // Please see the ATtiny25 datasheet for descriptions of these registers.
  TCCR0A = 0b01000010;  // COM0A1:0=01 to toggle OC0A on Compare Match
                        // COM0B1:0=00 to disconnect OC0B
                        // bits 3:2 are unused
                        // WGM01:00=10 for CTC Mode (WGM02=0 in TCCR0B)
  TCCR0B = 0b00000001;  // FOC0A=0 (no force compare)
                        // F0C0B=0 (no force compare)
                        // bits 5:4 are unused
                        // WGM2=0 for CTC Mode (WGM01:00=10 in TCCR0A)
                        // CS02:00=001 for divide by 1 prescaler (this starts Timer0)
  OCR0A = 104;  // to output 38,095.2KHz on OC0A (PB0, pin 5)

  // keep the IR going at 38KHz for 170 microseconds (which is slightly more than 6 periods of 38KHz)
  delay_ten_us(17);   // delay 170 microseconds

  // turn off Timer0 to stop 38KHz pulsing of IR
  TCCR0B = 0b00000000;  // CS02:CS00=000 to stop Timer0 (turn off IR emitter)
  TCCR0A = 0b00000000;  // COM0A1:0=00 to disconnect OC0A from PB0 (pin 5)
}

int main(void) {
  // disable the Watch Dog Timer (since we won't be using it, this will save battery power)
  MCUSR = 0b00000000;   // first step:   WDRF=0 (Watch Dog Reset Flag)
  WDTCR = 0b00011000;   // second step:  WDCE=1 and WDE=1 (Watch Dog Change Enable and Watch Dog Enable)
  WDTCR = 0b00000000;   // third step:   WDE=0
  // turn off power to the USI and ADC modules (since we won't be using it, this will save battery power)
  PRR = 0b00000011;
  // disable all Timer interrupts
  TIMSK = 0x00;         // setting a bit to 0 disables interrupts
  // set up the input and output pins (the ATtiny25 only has PORTB pins)
  DDRB = 0b00010111;    // setting a bit to 1 makes it an output, setting a bit to 0 makes it an input
                        //   PB5 (unused) is input
                        //   PB4 (Blue LED) is output
                        //   PB3 (IR detect) is input
                        //   PB2 (Green LED) is output
                        //   PB1 (Red LED) is output
                        //   PB0 (IR LED) is output
  PORTB = 0xFF;         // all PORTB output pins High (all LEDs off) -- (if we set an input pin High it activates a pull-up resistor, which we don't need, but don't care about either)
  // set up PB3 so that a logic change causes an interrupt (this will happen when the IR detector goes from seeing IR to not seeing IR, or from not seeing IR to seeing IR)
  GIMSK = 0b00100000;   // PCIE=1 to enable Pin Change Interrupts
  PCMSK = 0b00001000;   // PCINT3 bit = 1 to enable Pin Change Interrupts for PB3
  sei();                // enable microcontroller interrupts


  TCCR0A = 0b01000010;  // COM0A1:0=01 to toggle OC0A on Compare Match
                        // COM0B1:0=00 to disconnect OC0B
                        // bits 3:2 are unused
                        // WGM01:00=10 for CTC Mode (WGM02=0 in TCCR0B)
  TCCR0B = 0b00000001;  // FOC0A=0 (no force compare)
                        // F0C0B=0 (no force compare)
                        // bits 5:4 are unused
                        // WGM2=0 for CTC Mode (WGM01:00=10 in TCCR0A)
                        // CS02:00=001 for divide by 1 prescaler (this starts Timer0)
  OCR0A = 104;  // to output 38,095.2KHz on OC0A (PB0, pin 5)


  while(1) {
    for(unsigned int o = 0; o < ITE_MAX/ite; o++) {
      for(hue = 0; hue < HUE_MAX; hue += HUE_STEP) {
        /* get color value corresponding to current hue */
        hsb_to_color(hue, SAT, VAL, &color);
        for(unsigned int i = 0; i < ite; i++) {
          /* software PWM that shit */
          for(uint8_t pwm = 0; pwm < (1<<8)-1; pwm++) {
            if(pwm < color.rgb.r) { RED_ON }
            else { RED_OFF }
            if(pwm < color.rgb.g) { GRN_ON }
            else { GRN_OFF }
            if(pwm < color.rgb.b) { BLU_ON }
            else { BLU_OFF }
          }
        }
      }
    }
    ite += ITE_STEP;
    if(ite > ITE_MAX) { ite = ITE_MAX; }
  }
  while(1);
}
