/*
 *   HDSPMixer
 *    
 *   Copyright (C) 2003 Thomas Charbonnel (thomas@undata.org)
 *    
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma interface
#ifndef HDSPMixerInputs_H
#define HDSPMixerInputs_H

#include <FL/Fl_Group.H>
#include "HDSPMixerIOMixer.h"
#include "HDSPMixerButtons.h"
#include "HDSPMixerEmpty.h"

class HDSPMixerButtons;
class HDSPMixerIOMixer;
class HDSPMixerEmpty;

/*! \brief Collection of IOMixer's
 *
 * Groups together all inputs into the first strip
 * The Right Menus are incorporated in this class
 */
class HDSPMixerInputs:public Fl_Group
{
public:
    HDSPMixerButtons *buttons;
    HDSPMixerIOMixer *strips[HDSP_MAX_CHANNELS];
    HDSPMixerInputs(int x, int y, int w, int h, int nchannels);
    HDSPMixerEmpty *empty_aebi[2];
};

#endif

