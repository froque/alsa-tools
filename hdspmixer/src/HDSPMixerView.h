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
#ifndef HDSPMixerView_H
#define HDSPMixerView_H

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include "HDSPMixerWindow.h"
#include "pixmaps.h"

class HDSPMixerWindow;


/*! \brief Right Side Menu View
 *
 *  Draws checkmarks for Input,Output, Playback and Submix
 */
class HDSPMixerView:public Fl_Widget
{
private:
    HDSPMixerWindow *basew;
public:
    int input, playback, output, submix, submix_value;
    HDSPMixerView(int x, int y, int w, int h);
    void draw();
    int handle(int e);
};

#endif

