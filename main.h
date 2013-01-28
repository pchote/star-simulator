//*****************************************************************************
//  Copyright 2012, 2013 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#ifndef LIGHTBOX_MAIN_H
#define LIGHTBOX_MAIN_H

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

struct mode
{
    double freq;
    double mma;
    double phase;
};

struct output
{
    enum current_value current;
    double pwm_duty;
    uint8_t mode_count;
    struct mode modes[MAX_MODES];

    volatile uint16_t *ocr;
    volatile uint8_t *port;
    uint8_t mask;
};

#endif
