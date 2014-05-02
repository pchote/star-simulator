//*****************************************************************************
//  Copyright 2012 - 2014 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <string.h>
#include "main.h"
#include "cloudgen.h"
#include "simulation.h"
#include "usb.h"

//
// Each output channel has a configurable current source, and configurable
// pulse-width modulation duty cycle.  This provides a significant dynamic
// range of configurable intensities.
//

struct channel
{
    volatile uint16_t *ocr;
    volatile uint8_t *port;
    uint8_t mask;
};

// Hardware outputs
struct channel channels[4];

// Simulation output
static struct output outputs[4];
static struct cloudgen cloud;

// Simulation types
uint8_t simulation_count = 5;
struct simulation_parameters simulation[5];
uint8_t active_simulation = 0;

static void channel_set_duty(uint8_t i, double duty)
{
    *(channels[i].ocr) = (uint16_t)(0x03FF*duty);
}

static void channel_set_current(uint8_t i, uint8_t state)
{
    uint8_t masked = (state & 0x0F) << 4*(channels[i].mask == 0xF0);
    *channels[i].port &= ~channels[i].mask;
    *channels[i].port |= masked;
}

int main(void)
{
    // Initialize output channels
    // Use 10-bit PWM on timer1,3 to control two channels each
    TCCR1A |= _BV(WGM10) | _BV(WGM11) | _BV(COM1A1) | _BV(COM1B1);
    TCCR1B |= _BV(WGM12) | _BV(CS11) | _BV(CS10);
    DDRB |= _BV(PB5) | _BV(PB6);

    TCCR3A |= _BV(WGM30) | _BV(WGM31) | _BV(COM3A1) | _BV(COM3B1);
    TCCR3B |= _BV(WGM32) | _BV(CS31) | _BV(CS30);
    DDRE |= _BV(PE3) | _BV(PE4);

    // Set all pins to output
    DDRA = 0xFF;
    DDRD = 0xFF;

    channels[0] = (struct channel){ .ocr = &OCR3A, .port = &PORTA, .mask = 0x0F };
    channels[1] = (struct channel){ .ocr = &OCR3B, .port = &PORTA, .mask = 0xF0 };
    channels[2] = (struct channel){ .ocr = &OCR1B, .port = &PORTD, .mask = 0xF0 };
    channels[3] = (struct channel){ .ocr = &OCR1A, .port = &PORTD, .mask = 0x0F };

    // Configure timer0 with 64us ticks to update the output channels every 16ms
    TCCR0 = _BV(CS02) | _BV(CS01) | _BV(CS00);
    TIMSK |= _BV(TOIE0);

    simulation[0] = simulation_beating();
    simulation[1] = simulation_ec20058_realtime();
    simulation[2] = simulation_ec20058_fast();
    simulation[3] = simulation_crab_pulsar_slow();
    simulation[4] = simulation_test_ramp();

    // Initialize other components
    usb_initialize();
    select_simulation(eeprom_read_byte(MODE_EEPROM_OFFSET));

    // The output is updated via a timed interrupt configured in simulation_initialize,
    // and so we only need to poll USB in the main loop
    sei();
    for (;;)
        usb_tick();
}

static double tick_output(struct output *o, double dt)
{
    // Tick the simulation of the specified channel and return the current intensity
    switch (o->type)
    {
        case Sinusoidal:
        {
            struct sinusoid_variability *s = &o->sinusoid;

            // Increment mode phases and calculate new brightness
            double mma = 0;
            for (uint8_t j = 0; j < s->mode_count; j++)
            {
                struct sinusoid *m = &s->modes[j];
                m->phase += m->freq*dt;
                while (m->phase > 1)
                    m->phase -= 1;

                mma += m->mma*sin(2*M_PI*m->phase);
            }

            return (1 + mma/1000);
        }

        case Ramp:
        {
            struct ramp_variability *r = &o->ramp;

            r->accumulated += dt;

            if (r->accumulated > r->period)
                r->accumulated -= r->period;

            return r->accumulated / r->period;
        }

        case Gaussian:
        {
            struct gaussian_variability *g = &o->gaussian;

            g->accumulated += dt;

            if (g->accumulated > g->period)
                g->accumulated -= g->period;

            double phase = g->accumulated / g->period;
            double intensity = 0;
            for (uint8_t j = 0; j < g->mode_count; j++)
            {
                struct gaussian *p = &g->modes[j];
                double x = (phase - p->offset)/p->width;
                intensity += p->amplitude * exp(-x*x);
            }

            return intensity;
        }
        
        case Constant:
            return 1;
    }

    return 0;
}

void select_simulation(uint8_t simulation_type)
{
    // Sanity check input - reset to the first definition on error
    // Simulation IDs are 1-indexed to make user-friendlier ids.
    if (simulation_type == 0 || simulation_type > simulation_count)
    {
        select_simulation(1);
        return;
    }

    // Save choice
    active_simulation = simulation_type;
    eeprom_update_byte(MODE_EEPROM_OFFSET, simulation_type);

    // Clear existing parameters
    memset(&cloud, 0, sizeof(cloud));
    memset(outputs, 0, sizeof(outputs));

    // Load new parameters
    (simulation[simulation_type-1].initialize)(&cloud, outputs);

    // Initialize simulation
    cloudgen_init(&cloud);
    for (uint8_t i = 0; i < 4; i++)
    {
        channel_set_current(i, outputs[i].current);
        channel_set_duty(i, outputs[i].pwm_duty);
    }

    // Notify the user of the change
    usb_send_simulation_changed(simulation_type);
}

// Intensity update interrupt.
// Called every 16.32 ms +/- clock tolerance when timer0 overflows
ISR(TIMER0_OVF_vect)
{
    const double dt = 0.01632;

    // Calculate cloud attenuation
    double attenuation = cloudgen_step(&cloud, dt);

    // Update the four output channels
    for (uint8_t i = 0; i < 4; i++)
    {
        double intensity = tick_output(&outputs[i], dt);

        if (outputs[i].cloudy)
            intensity *= attenuation;

        channel_set_duty(i, intensity*outputs[i].pwm_duty);
    }
}