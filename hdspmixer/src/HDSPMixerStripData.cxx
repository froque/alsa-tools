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

#pragma implementation
#include "HDSPMixerStripData.h"

HDSPMixerStripData::HDSPMixerStripData() {
    dest = mute = solo = 0;
    /*! \bug Should  be HDSP_MAX_DEST and not 14 */
    for (int i = 0; i < 14; i++) {
        fader_pos[i] = pan_pos[i] = 0;
    }
}

