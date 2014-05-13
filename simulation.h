//*****************************************************************************
//  Copyright 2012 - 2014 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#ifndef LIGHTBOX_SIMULATION_H
#define LIGHTBOX_SIMULATION_H

struct simulation_parameters simulation_beating();
struct simulation_parameters simulation_ec20058_realtime();
struct simulation_parameters simulation_ec20058_realtime_cloud();
struct simulation_parameters simulation_ec20058_fast();
struct simulation_parameters simulation_ec20058_fast_cloud();
struct simulation_parameters simulation_crab_pulsar_slow();
struct simulation_parameters simulation_test_ramp();

#endif
