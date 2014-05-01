//*****************************************************************************
//  Copyright 2012 - 2014 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <math.h>
#include "main.h"
#include "cloudgen.h"

// Uncomment one of the following to set the simulation parameters
//#include "parameters_testramp.h"
#include "parameters_photexpt.h"

// Timer interval between timer ticks in seconds
// based on CPU speed and prescaler
struct output outputs[4];
struct cloudparams cloud;

static void set_pwm_duty(uint8_t i, double duty)
{
    *(outputs[i].ocr) = (uint16_t)(0x03FF*duty);
}

static void set_current(uint8_t i, uint8_t state)
{
    uint8_t masked = (state & 0x0F) << 4*(outputs[i].mask == 0xF0);
    *outputs[i].port &= ~outputs[i].mask;
    *outputs[i].port |= masked;
}

static void configure_output(uint8_t i, volatile uint16_t *ocr, volatile uint8_t *port, uint8_t mask)
{
    outputs[i].ocr = ocr;
    outputs[i].port = port;
    outputs[i].mask = mask;

    set_current(i, outputs[i].current);
    set_pwm_duty(i, outputs[i].pwm_duty);
}

int main(void)
{
    // Enable 10-bit PWM on timer1,3
    TCCR1A |= _BV(WGM10) | _BV(WGM11) | _BV(COM1A1) | _BV(COM1B1);
    TCCR1B |= _BV(WGM12) | _BV(CS11) | _BV(CS10);
    DDRB |= _BV(PB5) | _BV(PB6);

    TCCR3A |= _BV(WGM30) | _BV(WGM31) | _BV(COM3A1) | _BV(COM3B1);
    TCCR3B |= _BV(WGM32) | _BV(CS31) | _BV(CS30);
    DDRE |= _BV(PE3) | _BV(PE4);

    // Enable outputs
    DDRA = 0xFF;
    DDRD = 0xFF;

    set_parameters(outputs, &cloud);
    configure_output(0, &OCR3A, &PORTA, 0x0F);
    configure_output(1, &OCR3B, &PORTA, 0xF0);
    configure_output(2, &OCR1B, &PORTD, 0x0F);
    configure_output(3, &OCR1A, &PORTD, 0xF0);

    if (cloud.enabled)
        cloudgen_init(&cloud);

    // Set the timer tick to 64us
    TCCR0 = _BV(CS02) | _BV(CS01) | _BV(CS00);
    TIMSK |= _BV(TOIE0);
    sei();
    for (;;);
}

ISR(TIMER0_OVF_vect)
{
    const double dt = 0.01632;

    // Calculate cloud attenuation
    double attenuation = cloud.enabled ? cloudgen_step(dt) : 1;

    for (uint8_t i = 0; i < 4; i++)
    {
        struct output *o = &outputs[i];
        double intensity = 1.0;

        if (o->type == Sinusoidal && o->mode_count > 0)
        {
            // Increment mode phases
            for (uint8_t j = 0; j < o->mode_count; j++)
            {
                struct mode *m = &o->modes[j];
                m->phase += m->freq*dt;
                while (m->phase > 1)
                    m->phase -= 1;
            }

            // Calculate new brightness
            double mma = 0;
            for (uint8_t j = 0; j < o->mode_count; j++)
            {
                struct mode *m = &o->modes[j];
                mma += m->mma*sin(2*M_PI*m->phase);
            }

            intensity = (1 + mma/1000);
        }
        else if (o->type == Ramp)
        {
            o->accumulated += dt;

            if (o->accumulated > o->period)
                o->accumulated -= o->period;

            intensity = o->accumulated / o->period;
        }
        else if (o->type == Gaussian)
        {
            o->accumulated += dt;

            if (o->accumulated > o->period)
                o->accumulated -= o->period;

            double phase = o->accumulated / o->period;
            intensity = 0;

            for (uint8_t j = 0; j < o->mode_count; j++)
            {
                struct gaussian *p = &o->peaks[j];
                double x = (phase - p->offset)/p->width;
                intensity += p->amplitude * exp(-x*x);
            }
        }

        if (o->cloudy)
            intensity *= attenuation;

        set_pwm_duty(i, intensity*outputs[i].pwm_duty);
    }
}
