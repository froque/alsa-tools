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
#include "HDSPMixerAbout.h"


HDSPMixerAbout::HDSPMixerAbout(int w, int h, char const *label, HDSPMixerWindow *win):Fl_Double_Window(w, h, label)
{
    basew = win;
    text = new HDSPMixerAboutText(10,10,w-20,h-20);
    end();
    set_modal();
}

int HDSPMixerAbout::handle(int e) {
    switch (e) {
    case FL_PUSH:
        hide();
        return 1;
    default:
        return Fl_Window::handle(e);
    }
}
