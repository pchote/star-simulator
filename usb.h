//*****************************************************************************
//  Copyright 2012 - 2014 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#ifndef LIGHTBOX_USB_H
#define LIGHTBOX_USB_H

#include <stdarg.h>
#include <stdint.h>
#include "main.h"

void usb_initialize();
void usb_tick();

void usb_send_message_P(const char *string);
void usb_send_message_fmt_P(const char *fmt, ...);
void usb_send_raw(uint8_t *data, uint8_t length);
void usb_send_simulation_type(uint8_t index, struct simulation_parameters *params);
void usb_send_simulation_count(uint8_t total, uint8_t active);
void usb_send_simulation_changed(uint8_t index);

#endif
