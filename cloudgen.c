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
#include "main.h"

struct cloudparams *params;
double accumulated_time = 0;
uint8_t start = 0;

// A circular buffer of the 4 control points for catmull-rom spline interpolation
double points[4];

static double get_random(double min, double max)
{
    return min + rand() * (max - min) / RAND_MAX;
}

static int8_t wrap(int8_t i)
{
    while (i < 0)
        i += 4;

    while (i >= 4)
        i -= 4;

    return i;
}

static double p(int8_t i)
{
    return points[wrap(start + i)];
}

// Initialize first four parameters
void cloudgen_init(struct cloudparams *_params)
{
    params = _params;

    // Redundant, but this makes it clear that we want the same
    // pseudo-random sequence every power-up.
    srand(0);

    for (uint8_t i = 0; i < 4; i++)
        points[i] = params->initial_intensity;
}

double cloudgen_step(double dt)
{
    accumulated_time += dt;
    if (accumulated_time > params->period)
    {
        // Generate new control point value
        // Allow a +/- 5% intensity each step
        double cur = p(start);
        double next = cur + get_random(-params->velocity, params->velocity);

        // If the value is outside the allowed range, then reflect it back inside
        if (next > params->max_intensity)
            next = 2*params->max_intensity - next;
        if (next < params->min_intensity)
            next = 2*params->min_intensity - next;

        points[start] = next;

        // Increment control point
        start = wrap(start + 1);

        accumulated_time -= params->period;
    }

    // Evaluate the catmull-rom spline
    double t = accumulated_time / params->period;
    return p(1) + 0.5f*t*(p(2) - p(0) + t*((2*p(0) - 5*p(1) + 4*p(2) - p(3)) + t*(3*p(1) - 3*p(2) + p(3) - p(0))));
}