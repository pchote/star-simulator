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

// Initialize first four parameters
void cloudgen_init(struct cloudgen *cloud)
{
    for (uint8_t i = 0; i < 4; i++)
        cloud->points[i] = cloud->initial_intensity;
}

double cloudgen_step(struct cloudgen *cloud, double dt)
{
    if (!cloud->enabled)
        return 1;

    cloud->accumulated_time += dt;
    if (cloud->accumulated_time > cloud->period)
    {
        // Generate new control point value
        // Allow a +/- 5% intensity each step
        double cur = cloud->points[cloud->start];
        double next = cur + get_random(-cloud->velocity, cloud->velocity);

        // If the value is outside the allowed range, then reflect it back inside
        if (next > cloud->max_intensity)
            next = 2*cloud->max_intensity - next;
        if (next < cloud->min_intensity)
            next = 2*cloud->min_intensity - next;

        cloud->points[cloud->start] = next;

        // Increment control point
        cloud->start = wrap(cloud->start + 1);

        cloud->accumulated_time -= cloud->period;
    }

    // Evaluate the catmull-rom spline
    double t = cloud->accumulated_time / cloud->period;
    double p0 = cloud->points[wrap(cloud->start + 0)];
    double p1 = cloud->points[wrap(cloud->start + 1)];
    double p2 = cloud->points[wrap(cloud->start + 2)];
    double p3 = cloud->points[wrap(cloud->start + 3)];

    return p1 + 0.5f*t*(p2 - p0 + t*((2*p0 - 5*p1 + 4*p2 - p3) + t*(3*p1 - 3*p2 + p3 - p0)));
}