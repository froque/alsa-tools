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
#ifndef HDSPMixerPeak_H
#define HDSPMixerPeak_H

#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include "HDSPMixerIOMixer.h"
#include "HDSPMixerOutput.h"

class HDSPMixerIOMixer;
class HDSPMixerOutput;

/*! \brief Peak text box Widget
 *
 *  Draws the text for the box above fader and meter for Inputs, Playbacks and Outputs
 */
class HDSPMixerPeak:public Fl_Widget
{
private:
    int parent_iomixer, over;
    char text[10];
public:
    HDSPMixerPeak(int x, int y, int parenttype);
    void draw();
    void update(double maxlevel, int over);
};

#endif

