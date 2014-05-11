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
#include <math.h>
#include "cloudgen.h"
#include "main.h"

static uint16_t rand_value;
static uint8_t random_write_bit;

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

ISR(ADC_vect)
{
    // The random number is updated every XXX ms by setting successive
    // bits based on the relative voltage of the floating ADC0 and ADC1 inputs.
    rand_value ^= (ADCH > 128 ? 1 : 0) << random_write_bit;
    if (random_write_bit++ == 16)
        random_write_bit = 0;
}

// Initialize first four parameters
void cloudgen_init(struct cloudgen *cloud)
{
    for (uint8_t i = 0; i < 4; i++)
        cloud->points[i] = cloud->initial_intensity;

    // Enable ADC for random number generation
    // Set sample rate to 125khz
    ADCSRA |= _BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0);

    // Left-align output in ADCH, enable AVCC, do a high-gain differential measurement
    ADMUX |= _BV(ADLAR) | _BV(REFS1) | _BV(MUX3) | _BV(MUX0);

    // Enable ADC; enable free running mode; enable interrupt; start measuring
    ADCSRA |= _BV(ADEN)|_BV(ADFR)|_BV(ADIE)|_BV(ADSC);
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