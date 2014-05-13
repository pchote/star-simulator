//*****************************************************************************
//  Copyright 2012 - 2014 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#include <avr/pgmspace.h>
#include "simulation.h"
#include "main.h"

static const char beating_name[] PROGMEM = "Beating test signal.";
static const char beating_desc[] PROGMEM = "Two sinusoids, with periods of 20 and 25 seconds.";
static const uint16_t beating_exptime = 1000;
static void beating_init(struct cloudgen *cloud, struct output outputs[4])
{
    outputs[2] = (struct output) {
        .current = c50uA,
        .pwm_duty = 0.5,
        .type = Sinusoidal,
        .sinusoid = {
            .mode_count = 2,
            .modes = {
                {0.05, 100, 0},
                {0.04, 50, 0.5},
            }
        }
    };
}

struct simulation_parameters simulation_beating()
{
    return (struct simulation_parameters) {
        .name = beating_name,
        .desc = beating_desc,
        .exptime = beating_exptime,
        .initialize = beating_init
    };
}

static const char ec20058_realtime_name[] PROGMEM = "EC20058 simulation (real-time).";
static const char ec20058_realtime_desc[] PROGMEM = "Simulation of the white dwarf EC20058.";
static const uint16_t ec20058_realtime_exptime = 20000;
void ec20058_realtime_init(struct cloudgen *cloud, struct output outputs[4])
{
    outputs[2] = (struct output) {
        .current = c5uA,
        .pwm_duty = 0.5,
        .cloudy = true,
        .type = Sinusoidal,
        .sinusoid = {
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
        }
    };
}

struct simulation_parameters simulation_ec20058_realtime()
{
    return (struct simulation_parameters) {
        .name = ec20058_realtime_name,
        .desc = ec20058_realtime_desc,
        .exptime = ec20058_realtime_exptime,
        .initialize = ec20058_realtime_init
    };
}

static const char ec20058_realtime_cloud_name[] PROGMEM = "EC20058 simulation (cloudy; real-time).";
static const char ec20058_realtime_cloud_desc[] PROGMEM = "EC20058 and a constant comparison star on a cloudy night.";
static const uint16_t ec20058_realtime_cloud_exptime = 20000;
void ec20058_realtime_cloud_init(struct cloudgen *cloud, struct output outputs[4])
{
    *cloud = (struct cloudgen) {
        .enabled = true,
        .min_period = 30,
        .max_period = 300,
        .min_intensity = 0.5,
        .max_intensity = 1,
        .initial_intensity = 0.75
    };

    outputs[2] = (struct output) {
        .current = c5uA,
        .pwm_duty = 0.5,
        .cloudy = true,
        .type = Sinusoidal,
        .sinusoid = {
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
        }
    };

    outputs[3] = (struct output) {
        .current = c5uA,
        .pwm_duty = 0.8,
        .type = Constant,
        .cloudy = true,
    };
}

struct simulation_parameters simulation_ec20058_realtime_cloud()
{
    return (struct simulation_parameters) {
        .name = ec20058_realtime_cloud_name,
        .desc = ec20058_realtime_cloud_desc,
        .exptime = ec20058_realtime_cloud_exptime,
        .initialize = ec20058_realtime_cloud_init
    };
}

static const char ec20058_fast_name[] PROGMEM = "EC20058 simulation (10x faster).";
static const char ec20058_fast_desc[] PROGMEM = "Simulation of the white dwarf EC20058 with accelerated time.";
static const uint16_t ec20058_fast_exptime = 2000;
void ec20058_fast_init(struct cloudgen *cloud, struct output outputs[4])
{
    outputs[2] = (struct output) {
        .current = c50uA,
        .pwm_duty = 0.5,
        .cloudy = true,
        .type = Sinusoidal,
        .sinusoid = {
            .mode_count = 9,
            .modes = {
                {1903.50e-5, 1.57, 0.95},
                {2998.70e-5, 2.72, 0.97},
                {3489.00e-5, 1.32, 0.15},
                {3559.00e-5, 7.24, 0.99},
                {3893.20e-5, 6.40, 0.34},
                {4887.80e-5, 2.13, 0.44},
                {4902.20e-5, 1.80, 0.19},
                {5128.60e-5, 2.44, 0.99},
                {7452.20e-5, 1.22, 0.17},
            }
        }
    };
}

struct simulation_parameters simulation_ec20058_fast()
{
    return (struct simulation_parameters) {
        .name = ec20058_fast_name,
        .desc = ec20058_fast_desc,
        .exptime = ec20058_fast_exptime,
        .initialize = ec20058_fast_init
    };
}

static const char ec20058_fast_cloud_name[] PROGMEM = "EC20058 simulation (cloudy; 10x faster).";
static const char ec20058_fast_cloud_desc[] PROGMEM = "EC20058 and a constant comparison star on a cloudy night.";
static const uint16_t ec20058_fast_cloud_exptime = 2000;
void ec20058_fast_cloud_init(struct cloudgen *cloud, struct output outputs[4])
{
    *cloud = (struct cloudgen) {
        .enabled = true,
        .min_period = 3,
        .max_period = 30,
        .min_intensity = 0.5,
        .max_intensity = 1,
        .initial_intensity = 0.75
    };

    outputs[2] = (struct output) {
        .current = c50uA,
        .pwm_duty = 0.5,
        .cloudy = true,
        .type = Sinusoidal,
        .sinusoid = {
            .mode_count = 9,
            .modes = {
                {1903.50e-5, 1.57, 0.95},
                {2998.70e-5, 2.72, 0.97},
                {3489.00e-5, 1.32, 0.15},
                {3559.00e-5, 7.24, 0.99},
                {3893.20e-5, 6.40, 0.34},
                {4887.80e-5, 2.13, 0.44},
                {4902.20e-5, 1.80, 0.19},
                {5128.60e-5, 2.44, 0.99},
                {7452.20e-5, 1.22, 0.17},
            }
        }
    };

    outputs[3] = (struct output) {
        .current = c50uA,
        .pwm_duty = 0.8,
        .type = Constant,
        .cloudy = true,
    };
}

struct simulation_parameters simulation_ec20058_fast_cloud()
{
    return (struct simulation_parameters) {
        .name = ec20058_fast_cloud_name,
        .desc = ec20058_fast_cloud_desc,
        .exptime = ec20058_fast_cloud_exptime,
        .initialize = ec20058_fast_cloud_init
    };
}

static const char crab_pulsar_slow_name[] PROGMEM = "Crab pulsar simulation (100x slower).";
static const char crab_pulsar_slow_desc[] PROGMEM = "Simulation of the Crab pulsar, slowed to ~3s period.";
static const uint16_t crab_pulsar_slow_exptime = 100;
static void crab_pulsar_slow_init(struct cloudgen *cloud, struct output outputs[4])
{
    outputs[2] = (struct output) {
        .current = c50uA,
        .pwm_duty = 0.9,
        .cloudy = false,
        .type = Gaussian,
        .gaussian = {
            .period = 3.3689,
            .mode_count = 2,
            .modes = {
                {1.038, 0.2438, 0.07566},
                {0.3866, 0.6668, 0.1018},
            }
        }
    };
}

struct simulation_parameters simulation_crab_pulsar_slow()
{
    return (struct simulation_parameters) {
        .name = crab_pulsar_slow_name,
        .desc = crab_pulsar_slow_desc,
        .exptime = crab_pulsar_slow_exptime,
        .initialize = crab_pulsar_slow_init
    };
}

//  Linear ramp in each channel for calibrating intensities
static const char test_ramp_name[] PROGMEM = "Ramp test signal.";
static const char test_ramp_desc[] PROGMEM = "Ramps output channels from 0 to max over 17 seconds.";
static const uint16_t test_ramp_exptime = 100;
static void test_ramp_init(struct cloudgen *cloud, struct output outputs[4])
{
    outputs[0] = (struct output) {
        .current = c50uA,
        .pwm_duty = 1.0,
        .type = Ramp,
        .ramp = { .period = 17 }
    };

    outputs[1] = (struct output) {
        .current = c50uA,
        .pwm_duty = 1.0,
        .type = Ramp,
        .ramp = { .period = 17 }
    };

    outputs[2] = (struct output) {
        .current = c50uA,
        .pwm_duty = 1.0,
        .type = Ramp,
        .ramp = { .period = 17 }
    };

    outputs[3] = (struct output) {
        .current = c50uA,
        .pwm_duty = 1.0,
        .type = Ramp,
        .ramp = { .period = 17 }
    };
}

struct simulation_parameters simulation_test_ramp()
{
    return (struct simulation_parameters) {
        .name = test_ramp_name,
        .desc = test_ramp_desc,
        .exptime = test_ramp_exptime,
        .initialize = test_ramp_init
    };
}
