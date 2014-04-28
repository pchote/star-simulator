//*****************************************************************************
//  Linear ramp in each channel for calibrating intensities
//
//  Copyright 2012 - 2014 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#ifndef LIGHTBOX_TESTRAMP_PARAMS_H
#define LIGHTBOX_TESTRAMP_PARAMS_H

#include "main.h"

void set_parameters(struct output outputs[4], struct cloudparams *cloud)
{
    cloud->enabled = false;

    outputs[0] = (struct output) {
        .current = c50uA,
        .pwm_duty = 0.5,
        .type = Ramp,
        .period = 17,
    };

    outputs[1] = (struct output) {
        .current = c50uA,
        .pwm_duty = 0.5,
        .type = Ramp,
        .period = 17
    };

    outputs[2] = (struct output) {
        .current = c50uA,
        .pwm_duty = 0.5,
        .type = Ramp,
        .period = 17
    };

    outputs[3] = (struct output) {
        .current = c50uA,
        .pwm_duty = 0.5,
        .type = Ramp,
        .period = 17
    };
}

#endif
