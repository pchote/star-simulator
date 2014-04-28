//*****************************************************************************
//  VUW undergraduate photometry experiment parameters
//
//  Copyright 2012 - 2014 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#ifndef LIGHTBOX_PHOTEXPT_PARAMS_H
#define LIGHTBOX_PHOTEXPT_PARAMS_H

#include "main.h"

void set_parameters(struct output outputs[4], struct cloudparams *cloud)
{
    *cloud = (struct cloudparams) {
        .enabled = true,

        .period = 300,
        .velocity = 0.05,
        .min_intensity = 0.5,
        .max_intensity = 1,
        .initial_intensity = 0.75
    };

    // Part 3: Crab pulsar
    outputs[0] = (struct output) {
        .current = c50uA,
        .pwm_duty = 0.5,
        .cloudy = false,
        .type = Gaussian,
        .period = 3.3689,
        .mode_count = 2,
        .peaks = {
            {1.038, 0.2438, 0.07566},
            {0.3866, 0.6668, 0.1018},
        }
    };

    // Part 1: Beating signals
    outputs[1] = (struct output) {
        .current = c50uA,
        .pwm_duty = 0.5,
        .cloudy = false,
        .type = Sinusoidal,
        .mode_count = 2,
        .modes = {
            {0.05, 100, 0},
            {0.04, 50, 0.5},
        }
    };

    // Part 2: Comparison + Cloud
    outputs[2] = (struct output) {
        .current = c5uA,
        .pwm_duty = 0.8,
        .type = Constant,
        .cloudy = true,
    };

    // Part 2: EC20058 + Cloud
    outputs[3] = (struct output) {
        .current = c5uA,
        .pwm_duty = 0.5,
        .cloudy = true,
        .type = Sinusoidal,
        .mode_count = 9,
        .modes = {
            {1903.50e-6, 1.57, 0.95},
            {2998.70e-6, 2.72, 0.97},
            {3489.00e-6, 1.32, 0.15},
            {3559.00e-6, 7.24, 0.99},
            {3893.20e-6, 6.40, 0.34},
            {4887.80e-6, 2.13, 0.44},
            {4902.20e-6, 1.80, 0.19},
            {5128.60e-6, 2.44, 0.99},
            {7452.20e-6, 1.22, 0.17},
        }
    };
}

#endif
