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
#ifndef HDSPMixerCard_H
#define HDSPMixerCard_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <alsa/asoundlib.h>
#include <alsa/sound/hdsp.h>
#include <alsa/sound/hdspm.h>
#include "defines.h"
#include "channelmap.h"
#include "HDSPMixerWindow.h"

/* temporary workaround until hdsp.h (HDSP_IO_Type gets fixed */
#ifndef RPM
# define RPM	4
#endif

class HDSPMixerWindow;

class HDSPMixerCard
{
private:
    snd_ctl_t *cb_handle;
    snd_async_handler_t *cb_handler;
    char *channel_map_input;    /*!< will be used in ALSA calls: snd_ctl_elem_value_set_integer */
    char *channel_map_playback; /*!< will be used in ALSA calls: snd_ctl_elem_value_set_integer */
    char *dest_map;             /*!< will be used in ALSA calls: snd_ctl_elem_value_set_integer */
    int playbacks_offset;       /*!< this has to do with the way the kernel driver reads and writes gains */
    void getAeb();              /*!< gets information about AEB (analog expansion boards) */

public:
    HDSPMixerWindow *basew;
    char name[6];               /*!< hw:%i */
    std::string cardname;       /*!< shortname in main.c */
    int channels_input;         /*!< number of input channels */
    int channels_playback;      /*!< number of playback channels */
    int channels_output;        /*!< number of output channels */
    int type;                   /*!< H9632, Multiface, Digiface, RPM, H9652, H9632,  HDSPeMADI, HDSPeAIO, HDSP_AES, HDSPeRayDAT */
    int last_preset;            /*!< Last activated preset before switching to another card */
    int last_dirty;             /*!< Last dirty flag before switching to another card */

    char *meter_map_input;      /*!< used in readregister_cb for meters peak and rms values */
    char *meter_map_playback;   /*!< used in readregister_cb for meters peak and rms values */
    int speed_mode;             /*!< ADAT Speed: SS, DS, QS */
    hdsp_9632_aeb_t h9632_aeb;  /*!< analog expansion boards for 9632*/

    HDSPMixerCard(int cardtype, int id, char *shortname);
    void setMode(int mode);     /*!< Sets speed mode to variable speed_mode */  /* fixme: Most of this function seems stupid to be in this class. move to HDSPMixerWindows */
    int initializeCard(HDSPMixerWindow *w); /*!< initializes the card. This should be done in the constructor, not here */ /*fixme: move this to the constructor */
    int getSpeed();             /*!< access card to get current speed */
    int getAutosyncSpeed();     /*!< access card to get current Auto sync speed */  /* fixme: this should be private */
    void adjustSettings();      /*!< sets class variables: channels_input, channels_playback, channels_output, channel_map_input, channel_map_playback, dest_map, meter_map_input */ /* fixme: this should be private */
    void setGain(int in, int out, int value); /*!< wrapper around Mixer ctl interface */
    void resetMixer();          /*!< clears all gains */
    void getPeakRmsMadi(struct hdspm_peak_rms *hdspm_peak_rms); /*!< updates Peak and RMS values for MADI devices */
    void getPeakRms(hdsp_peak_rms_t *hdsp_peak_rms);            /*!< updates Peak and RMS values for non-MADI devices */
    void setInput(int in_idx, int out_idx, int left_value,int right_value);
    void setPlayback(int in_idx, int out_idx, int left_value,int right_value);
};

#endif

