//*****************************************************************************
//  Parameters for EC04207-4748
//
//  Copyright 2012, 2013 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************


#ifndef LIGHTBOX_EC04207_PARAMS_H
#define LIGHTBOX_EC04207_PARAMS_H

#include "main.h"

void set_parameters(struct output outputs[4])
{
    outputs[0] = (struct output) {
        .current = cDisabled,
        .pwm_duty = 0.7,
        .mode_count = 0
    };

    outputs[1] = (struct output) {
        .current = c5uA,
        .pwm_duty = 0.5,
        .mode_count = 12,
        .modes = {
            {2236.25E-6, 38.23, 1.86},
            {2361.44E-6,  9.20, 6.10},
            {2972.68E-6,  4.96, 5.59},
            {4472.52E-6, 10.63, 2.44},
            {4597.94E-6,  2.00, 0.43},
            {5208.83E-6,  2.53, 0.01},
            {6708.84E-6,  4.16, 3.07},
            {6830.90E-6,  1.47, 1.69},
            {7445.37E-6,  1.01, 0.19},
            {8945.05E-6,  1.74, 3.86},
            {2918.25E-6,  2.13, 5.07},
            {2233.12E-6,  2.35, 2.19},
        }
    };

    outputs[2] = (struct output) {
        .current = c50uA,
        .pwm_duty = 0.5,
        .mode_count = 0
    };

    outputs[3] = (struct output) {
        .current = cDisabled,
        .pwm_duty = 0.5,
        .mode_count = 0
    };
}

#endif
