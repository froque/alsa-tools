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
#ifndef HDSPMixerStripData_H
#define HDSPMixerStripData_H

#include "defines.h"

/*! \brief Data for Input and Playback
 *
 * Mute, Solo, Pan, Fader and destination of an Input or Playback
 */
class HDSPMixerStripData
{
public:
    int mute;
    int solo;
    int pan_pos[HDSP_MAX_DEST];
    int fader_pos[HDSP_MAX_DEST];
    int dest;
    HDSPMixerStripData();
};

#endif

