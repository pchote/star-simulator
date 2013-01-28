//*****************************************************************************
//  Fast modulating parameters (~mHz) for testing sub-second exposures
//
//  Copyright 2012, 2013 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#ifndef LIGHTBOX_SUBSEC_PARAMS_H
#define LIGHTBOX_SUBSEC_PARAMS_H

#include "main.h"

void set_parameters(struct output outputs[4])
{
    outputs[0] = (struct output) {
        .current = c500uA,
        .pwm_duty = 0.7,
        .mode_count = 0
    };

    outputs[1] = (struct output) {
        .current = c500uA,
        .pwm_duty = 0.1,
        .mode_count = 4,
        .modes = {
            {0.1, 200, 0},
            {0.15, 50, 1.2},
            {0.05, 100, 3},
            {0.099, 120, 1.8},
        }
    };

    outputs[2] = (struct output) {
        .current = c5mA,
        .pwm_duty = 0.125,
        .mode_count = 0
    };

    outputs[3] = (struct output) {
        .current = c500uA,
        .pwm_duty = 0.5,
        .mode_count = 0
    };
}

#endif
