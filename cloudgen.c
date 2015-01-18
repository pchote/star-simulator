//*****************************************************************************
//  Copyright 2012 - 2014 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <math.h>
#include "cloudgen.h"
#include "main.h"

static uint16_t rand_value;

static double lerp(double a, double b, double numerator, double denominator)
{
    return a + numerator * (b - a) / denominator;
}

static int8_t wrap(int8_t i)
{
    while (i < 0)
        i += 4;

    while (i >= 4)
        i -= 4;

    return i;
}

ISR(WDT_vect)
{
    uint8_t rnd = TCNT2;

    // Rotate the existing value a random number of places
    uint8_t shift = rnd & 0x0F;    
    rand_value = (rand_value << shift) | (rand_value >> 16 - shift);

    // Mix with the counter value
    rand_value ^= rnd;
}

// Initialize first four parameters
void cloudgen_init(struct cloudgen *cloud)
{
    for (uint8_t i = 0; i < 4; i++)
        cloud->points[i] = cloud->initial_intensity;

    // Use the watchdog timer to trigger an interrupt every 16ms.
    // This interval is measured using a separate oscillator to the main clock
    // and so we can use the relative clock skew (via timer2) to generate a
    // random bit.
    MCUSR = 0;
    WDTCSR |= _BV(WDCE) | _BV(WDE);
    WDTCSR = _BV(WDIE);
    TCCR2B = _BV(CS20);
}

double cloudgen_step(struct cloudgen *cloud, double dt)
{
    if (!cloud->enabled)
        return 1;

    cloud->accumulated_time += dt;

    if (cloud->accumulated_time > cloud->next_period)
    {
        // Generate new control point value
        double next = lerp(cloud->min_intensity, cloud->max_intensity, rand_value, UINT16_MAX);

        // Heavily weight the previous points
        cloud->points[cloud->start] = (2 * cloud->points[wrap(cloud->start + 1)] + 2 * cloud->points[wrap(cloud->start + 2)] + 2 * cloud->points[wrap(cloud->start + 3)] + next) / 7;

        // Increment control point
        cloud->start = wrap(cloud->start + 1);

        cloud->accumulated_time -= cloud->next_period;
        cloud->next_period = lerp(cloud->min_period, cloud->max_period, rand_value, UINT16_MAX);
    }

    // Evaluate the catmull-rom spline
    double p0 = cloud->points[wrap(cloud->start + 0)];
    double p1 = cloud->points[wrap(cloud->start + 1)];
    double p2 = cloud->points[wrap(cloud->start + 2)];
    double p3 = cloud->points[wrap(cloud->start + 3)];

    return lerp(p0 + p1, p2 + p3, cloud->accumulated_time, cloud->next_period) / 2;
}