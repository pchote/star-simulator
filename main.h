//*****************************************************************************
//  Copyright 2012 - 2014 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#ifndef LIGHTBOX_MAIN_H
#define LIGHTBOX_MAIN_H

#include <stdbool.h>
#include <stdint.h>
#include "simulation.h"

// Where the active configuration mode is stored
#define MODE_EEPROM_OFFSET (uint8_t *)(0x00)

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

struct sinusoid
{
    double freq;
    double mma;
    double phase;
};

// Maximum number of pulsation modes per output
#define MAX_MODES 10

struct sinusoid_variability
{
    uint8_t mode_count;
    struct sinusoid modes[MAX_MODES];
};

struct gaussian
{
    double amplitude;
    double offset;
    double width;
};

struct gaussian_variability
{
    uint8_t mode_count;
    struct gaussian modes[MAX_MODES];
    double period;
    double accumulated;
};

struct ramp_variability
{
    double period;
    double accumulated;
};

struct output
{
    enum current_value current;
    double pwm_duty;
    bool cloudy;

    enum variability_type type;
    union
    {
        struct sinusoid_variability sinusoid;
        struct gaussian_variability gaussian;
        struct ramp_variability ramp;
    };
};

struct cloudgen
{
    bool enabled;
    double period;
    double velocity;
    double min_intensity;
    double max_intensity;
    double initial_intensity;

    double accumulated_time;
    uint8_t start;

    // A circular buffer of the 4 control points for catmull-rom spline interpolation
    double points[4];
};

struct simulation_parameters
{
    const char *name;
    const char *desc;
    uint16_t exptime;
    void (*initialize)(struct cloudgen *, struct output *);
};

extern uint8_t simulation_count;
extern struct simulation_parameters simulation[];
extern uint8_t active_simulation;
void select_simulation(uint8_t simulation_type);

#endif
