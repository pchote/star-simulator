//*****************************************************************************
//  Copyright 2012 - 2014 Paul Chote
//  This file is part of lightbox, which is free software. It is made available
//  to you under version 3 (or later) of the GNU General Public License, as
//  published by the Free Software Foundation and included in the LICENSE file.
//*****************************************************************************

#ifndef LIGHTBOX_CLOUDGEN_H
#define LIGHTBOX_CLOUDGEN_H

#include "main.h"

void cloudgen_init(struct cloudparams *p);
float cloudgen_step(float dt);

#endif