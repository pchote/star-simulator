//*****************************************************************************
//  Copyright 2012 - 2014 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#ifndef LIGHTBOX_MAIN_H
#define LIGHTBOX_MAIN_H

#include <stdbool.h>

// Maximum number of pulsation modes per output
#define MAX_MODES 20

enum current_value
{
    cDisabled = 0,
    c5uA = 1,
    c50uA = 2,
    c500uA = 4,
    c5mA = 8
};

enum variability_type
{
    Constant = 0,
    Sinusoidal = 1,
    Gaussian = 2,
    Ramp = 3
};

struct mode
{
    double freq;
    double mma;
    double phase;
};

struct gaussian
{
    double amplitude;
    double offset;
    double width;
};

struct output
{
    enum current_value current;
    double pwm_duty;
    bool cloudy;
    enum variability_type type;

    // Used by sinusoidal and gaussian
    uint8_t mode_count;

    union
    {
        struct mode modes[MAX_MODES];
        struct gaussian peaks[MAX_MODES];
    };

    // Used by gaussian and ramp
    double period;
    double accumulated;

    volatile uint16_t *ocr;
    volatile uint8_t *port;
    uint8_t mask;
};


struct cloudparams
{
    bool enabled;
    double period;
    double velocity;
    double min_intensity;
    double max_intensity;
    double initial_intensity;
};

#endif
