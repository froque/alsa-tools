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
 *
 * @version 04-12-2009 [FF]
 *          - updated deprecated fl_ask calls
 *
 */

#pragma implementation
#include "HDSPMixerWindow.h"

/* header used in .mix file */
const char header[] = "HDSPMixer v1";

static void readregisters_cb(void *arg)
{
    hdsp_peak_rms_t hdsp_peak_rms;
    struct hdspm_peak_rms hdspm_peak_rms;
    bool isMADI = false;
    uint32_t *input_peaks, *playback_peaks, *output_peaks;
    uint64_t *input_rms, *playback_rms, *output_rms;
    
    HDSPMixerWindow *w = (HDSPMixerWindow *)arg;

    if (!w->visible()) {
        Fl::add_timeout(0.03, readregisters_cb, w);
        return;
    }
    
    int type = w->cards[w->current_card]->type;
    if ((HDSPeMADI == type) || (HDSPeAIO == type) || (HDSP_AES == type) || (HDSPeRayDAT == type)) {
        isMADI = true;
        w->cards[w->current_card]->getPeakRmsMadi(&hdspm_peak_rms);
    } else {
        w->cards[w->current_card]->getPeakRms(&hdsp_peak_rms);
    }


    if (isMADI) {
        // check for speed change
        if (hdspm_peak_rms.speed != w->cards[w->current_card]->speed_mode) {
            w->cards[w->current_card]->setMode(hdspm_peak_rms.speed);
            w->updateMode();
        }
        input_peaks = hdspm_peak_rms.input_peaks;
        playback_peaks = hdspm_peak_rms.playback_peaks;
        output_peaks = hdspm_peak_rms.output_peaks;

        input_rms = hdspm_peak_rms.input_rms;
        playback_rms = hdspm_peak_rms.playback_rms;
        output_rms = hdspm_peak_rms.output_rms;
    } else {
        /* speed changes on non-MADI are already handled via alsactl_cb and
         * getSpeed(), but the metering structs differ.
         */
        input_peaks = hdsp_peak_rms.input_peaks;
        playback_peaks = hdsp_peak_rms.playback_peaks;
        output_peaks = hdsp_peak_rms.output_peaks;

        input_rms = hdsp_peak_rms.input_rms;
        playback_rms = hdsp_peak_rms.playback_rms;
        output_rms = hdsp_peak_rms.output_rms;
    }

    /* update the meter */
    if (w->inputs->buttons->input) {
        for (int i = 0; i < w->cards[w->current_card]->channels_input; ++i) {
            w->inputs->strips[i]->meter->update(input_peaks[(w->cards[w->current_card]->meter_map_input[i])] & 0xffffff00,
                                                input_peaks[(w->cards[w->current_card]->meter_map_input[i])] & 0xf,
                                                input_rms[(w->cards[w->current_card]->meter_map_input[i])]);
        }
    }

    if (w->inputs->buttons->playback) {
        for (int i = 0; i < w->cards[w->current_card]->channels_playback; ++i) {
            w->playbacks->strips[i]->meter->update(playback_peaks[(w->cards[w->current_card]->meter_map_playback[i])] & 0xffffff00,
                                                   playback_peaks[(w->cards[w->current_card]->meter_map_playback[i])] & 0xf,
                                                   playback_rms[(w->cards[w->current_card]->meter_map_playback[i])]);
        }
    }

    if (w->inputs->buttons->output) {
        for (int i = 0; i < w->cards[w->current_card]->channels_output; ++i) {
            w->outputs->strips[i]->meter->update(output_peaks[(w->cards[w->current_card]->meter_map_playback[i])] & 0xffffff00,
                                                 output_peaks[(w->cards[w->current_card]->meter_map_playback[i])] & 0xf,
                                                 output_rms[(w->cards[w->current_card]->meter_map_playback[i])]);
        }
    }

    Fl::add_timeout(0.03, readregisters_cb, w);
}


static void exit_cb(Fl_Widget *widget, void *arg)
{
    HDSPMixerWindow *w = (HDSPMixerWindow *)arg;
    if (w->dirty) {
        if (!fl_choice("There are unsaved changes, quit anyway ?", "Return", "Quit", NULL)) return;
    }
    exit(EXIT_SUCCESS);
}

static void view_cb(Fl_Widget *widget, void *arg)
{
    const Fl_Menu_Item *item = ((Fl_Menu_ *)widget)->mvalue();
    HDSPMixerWindow *w = (HDSPMixerWindow *)arg;
    if (!strncmp(item->label(), "Input", 5)) {
        if (w->inputs->buttons->view->input) {
            w->inputs->buttons->view->input = 0;
        } else {
            w->inputs->buttons->view->input = 1;
        }
    }
    if (!strncmp(item->label(), "Playback", 8)) {
        if (w->inputs->buttons->view->playback) {
            w->inputs->buttons->view->playback = 0;
        } else {
            w->inputs->buttons->view->playback = 1;
        }
    }
    if (!strncmp(item->label(), "Output", 6)) {
        if (w->inputs->buttons->view->output) {
            w->inputs->buttons->view->output = 0;
        } else {
            w->inputs->buttons->view->output = 1;
        }
    }
    w->checkState();
    w->reorder();
}

static void submix_cb(Fl_Widget *widget, void *arg)
{
    HDSPMixerWindow *w = (HDSPMixerWindow *)arg;
    if (w->inputs->buttons->view->submix) {
        w->inputs->buttons->view->submix = 0;
        w->unsetSubmix();
    } else {
        w->inputs->buttons->view->submix = 1;
        w->setSubmix(w->inputs->buttons->view->submix_value);
    }
    w->checkState();
    w->inputs->buttons->view->redraw();
}

static void dirty_cb(void *arg)
{
    HDSPMixerWindow *w = (HDSPMixerWindow *)arg;
    if (!w->inputs->buttons->presets->saving) {
        if (w->inputs->buttons->presets->presetmask == (int)pow(2, w->inputs->buttons->presets->preset-1)) {
            w->inputs->buttons->presets->presetmask = 0;
        } else {
            w->inputs->buttons->presets->presetmask = (int)pow(2, w->inputs->buttons->presets->preset-1);
        }
        w->inputs->buttons->presets->redraw();
    }
    if (w->dirty) {
        Fl::add_timeout(0.3, dirty_cb, arg);
    } else {
        w->inputs->buttons->presets->presetmask = (int)pow(2, w->inputs->buttons->presets->preset-1);
    }
}

static void setup_cb(Fl_Widget *widget, void *arg)
{
    HDSPMixerWindow *w = (HDSPMixerWindow *)arg;
    w->setup->show();
}

static void about_cb(Fl_Widget *widget, void *arg)
{
    HDSPMixerWindow *w = (HDSPMixerWindow *)arg;
    w->about->show();
}

static void open_cb(Fl_Widget *widget, void *arg)
{
    HDSPMixerWindow *w = (HDSPMixerWindow *)arg;
    if (!(w->file_name = fl_file_chooser("Choose a file to load presets from :", "HDSPMixer preset file (*.mix)", NULL, 0))) return;
    w->load();
}

static void save_cb(Fl_Widget *widget, void *arg)
{
    HDSPMixerWindow *w = (HDSPMixerWindow *)arg;
    if (w->file_name == NULL) {
        if (!(w->file_name = fl_file_chooser("Choose a file to save presets to :", "HDSPMixer preset file (*.mix)", NULL, 0))) return;
    }
    w->save();
    w->setTitleWithFilename();
}

static void make_default_cb(Fl_Widget *widget, void *arg)
{
    HDSPMixerWindow *w = (HDSPMixerWindow *)arg;
    if (w->file_name) {
        w->prefs->set("default_file", w->file_name);
        w->prefs->flush();
    } else {
        fl_alert("Please save to a file before setting to default");
    }
}

static void restore_defaults_cb(Fl_Widget *widget, void *arg)
{
    HDSPMixerWindow *w = (HDSPMixerWindow *)arg;
    int i = 0;
    if (w->dirty) {
        if (!fl_choice("There are unsaved changes, restore factory settings anyway ?", "Don't restore", "Restore them", NULL)) return;
    }
    w->prefs->deleteEntry("default_file");
    w->prefs->flush();
    w->file_name = NULL;
    w->setTitleWithFilename();
    w->cards[w->current_card]->resetMixer();
    while (i < MAX_CARDS && w->cards[i] != NULL) {
        w->restoreDefaults(i++);
    }
    w->inputs->buttons->presets->preset_change(1);
}


static void save_as_cb(Fl_Widget *widget, void *arg)
{
    HDSPMixerWindow *w = (HDSPMixerWindow *)arg;
    if (!(w->file_name = fl_file_chooser("Choose a file to save presets to :", "HDSPMixer preset file (*.mix)", NULL, 0))) return;
    w->save();
}

static void atclose_cb(Fl_Window *w, void *arg)
{
    if (strncmp("HDSPMixer", w->label(), 9) == 0) {
        if (((HDSPMixerWindow *)w)->dirty) {
            if (!fl_choice("There are unsaved changes, quit anyway ?", "Don't quit", "Quit", NULL)) return;
        }
        exit(EXIT_SUCCESS);
    }
    w->hide();
}

static int handler_cb(int event)
{
    HDSPMixerWindow *w = NULL;
    Fl_Window *fl_win = Fl::first_window();
    while (1) {
        if (fl_win->label()) {
            if (strncmp("HDSPMixer", fl_win->label(), 9) == 0) {
                w = (HDSPMixerWindow *)fl_win;
                break;
            }
        }
        if ((fl_win = Fl::next_window(fl_win))) return 0;
    }
    if (!w) return 0;
    int key = Fl::event_key();
    switch (event) {
    case FL_SHORTCUT:
        if (key == FL_Escape) {
            if (w->dirty) {
                if (!fl_choice("There are unsaved changes, quit anyway ?", "Don't quit", "Quit", NULL)) return 1;
            }
            exit(EXIT_SUCCESS);
        }
        if (!w->setup->visible()) {
            if (key == 'r' || key == 'R') {
                /* numbers should show peak values */
                w->setup->numbers_val = 0;
                w->checkState();
                return 1;
            } else if (key == 'e' || key == 'E') {
                /* numbers should show rms values */
                w->setup->numbers_val = 1;
                w->checkState();
                return 1;
            }
            if (key == '0' || key == '0'+FL_KP) {
                /* rms +0dB */
                w->setup->rmsplus3_val = 0;
                w->checkState();
                return 1;
            } else 	if (key == '3' || key == '3'+FL_KP) {
                /* rms +3dB */
                w->setup->rmsplus3_val = 1;
                w->checkState();
                return 1;
            }
            if (key == '4' || key == '4'+FL_KP) {
                /* meter range is 40 dB */
                w->setup->level_val = 0;
                w->checkState();
                return 1;
            } else 	if (key == '6' || key == '6'+FL_KP) {
                /* meter range is 60 dB */
                w->setup->level_val = 1;
                w->checkState();
                return 1;
            }
        }
        break;
    default:
        return 0;
    }
    return 0;
}

void HDSPMixerWindow::save() 
{
    const int pan_array_size =
            sizeof(inputs->strips[0]->data[0][0][0]->pan_pos) /
            sizeof(inputs->strips[0]->data[0][0][0]->pan_pos[0]);
    /* MixerStripData defines pan_pos[HDSP_MAX_DEST], but just in case this
     * will ever change, let's detect it early and fail safely instead of
     * reading/writing garbage from/to preset files
     */
    assert (HDSP_MAX_DEST == pan_array_size);

    /* also make sure that fader_pos[] has the same size as pan_pos. This comes
     * naturally, but just to be sure.
     */
    assert (pan_array_size ==
            sizeof(inputs->strips[0]->data[0][0][0]->fader_pos) /
            sizeof(inputs->strips[0]->data[0][0][0]->fader_pos[0]));


    FILE *file;

    if ((file = fopen(file_name, "w")) == NULL) {
        fl_alert("Error opening file %s for saving", file_name);
    }
    if (dirty) {
        inputs->buttons->presets->save_preset(current_preset+1);
    }
    /* since hdspmixer 1.11, we also store the meter level settings. Indicate
     * the new on-disk structure via a small header */
    if (fwrite((void *)&header, sizeof(char), sizeof(header), file) !=
            sizeof(header)) {
        goto save_error;
    }

    for (int speed = 0; speed < 3; ++speed) {
        for (int card = 0; card < MAX_CARDS; ++card) {
            /*!< \bug should be NUM_PRESETS or NUM_PRESETS-1 */
            for (int preset = 0; preset < 8; ++preset) {
                for (int channel = 0; channel < HDSP_MAX_CHANNELS; ++channel) {
                    /* inputs pans and volumes */
                    if (fwrite((void *)&(inputs->strips[channel]->data[card][speed][preset]->pan_pos[0]), sizeof(int), pan_array_size, file) != pan_array_size) {
                        goto save_error;
                    }
                    if (fwrite((void *)&(inputs->strips[channel]->data[card][speed][preset]->fader_pos[0]), sizeof(int), pan_array_size, file) != pan_array_size) {
                        goto save_error;
                    }
                    /* playbacks pans and volumes */
                    if (fwrite((void *)&(playbacks->strips[channel]->data[card][speed][preset]->pan_pos[0]), sizeof(int), pan_array_size, file) != pan_array_size) {
                        goto save_error;
                    }
                    if (fwrite((void *)&(playbacks->strips[channel]->data[card][speed][preset]->fader_pos[0]), sizeof(int), pan_array_size, file) != pan_array_size) {
                        goto save_error;
                    }
                    /* inputs mute/solo/dest */
                    if (fwrite((void *)&(inputs->strips[channel]->data[card][speed][preset]->mute), sizeof(int), 1, file) != 1) {
                        goto save_error;
                    }
                    if (fwrite((void *)&(inputs->strips[channel]->data[card][speed][preset]->solo), sizeof(int), 1, file) != 1) {
                        goto save_error;
                    }
                    if (fwrite((void *)&(inputs->strips[channel]->data[card][speed][preset]->dest), sizeof(int), 1, file) != 1) {
                        goto save_error;
                    }
                    /* playbacks mute/solo/dest */
                    if (fwrite((void *)&(playbacks->strips[channel]->data[card][speed][preset]->mute), sizeof(int), 1, file) != 1) {
                        goto save_error;
                    }
                    if (fwrite((void *)&(playbacks->strips[channel]->data[card][speed][preset]->solo), sizeof(int), 1, file) != 1) {
                        goto save_error;
                    }
                    if (fwrite((void *)&(playbacks->strips[channel]->data[card][speed][preset]->dest), sizeof(int), 1, file) != 1) {
                        goto save_error;
                    }
                    /* outputs volumes */
                    if (fwrite((void *)&(outputs->strips[channel]->data[card][speed][preset]->fader_pos), sizeof(int), 1, file) != 1) {
                        goto save_error;
                    }

                }
                /* Lineouts */
                if (fwrite((void *)&(outputs->strips[HDSP_MAX_CHANNELS]->data[card][speed][preset]->fader_pos), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(outputs->strips[HDSP_MAX_CHANNELS+1]->data[card][speed][preset]->fader_pos), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                /* Global settings */
                if (fwrite((void *)&(data[card][speed][preset]->input), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(data[card][speed][preset]->output), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(data[card][speed][preset]->playback), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(data[card][speed][preset]->submix), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(data[card][speed][preset]->submix_value), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(data[card][speed][preset]->solo), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(data[card][speed][preset]->mute), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(data[card][speed][preset]->last_destination), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(data[card][speed][preset]->rmsplus3), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(data[card][speed][preset]->numbers), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(data[card][speed][preset]->over), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(data[card][speed][preset]->level), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
                if (fwrite((void *)&(data[card][speed][preset]->rate), sizeof(int), 1, file) != 1) {
                    goto save_error;
                }
            }
        }
    }
    fclose(file);
    return;
save_error:
    fclose(file);
    fl_alert("Error saving presets to file %s", file_name);
    return;
}

void HDSPMixerWindow::load()
{
    FILE *file;
    if ((file = fopen(file_name, "r")) == NULL) {
        int i = 0;
        fl_alert("Error opening file %s for reading", file_name);
        while (i < MAX_CARDS && cards[i] != NULL) {
            restoreDefaults(i++);
        }
        inputs->buttons->presets->preset_change(1);
        return;
    }

    /* check for new ondisk format */
    char buffer[sizeof(header)];
    bool ondisk_v1 = false;
    int pan_array_size = 14; /* old (pre 1.0.24) HDSP_MAX_DEST */
    int channels_per_card = 26; /* old (pre 1.0.24) HDSP_MAX_CHANNELS */

    if (fread(&buffer, sizeof(char), sizeof(buffer), file) != sizeof(buffer)) {
        goto load_error;
    }
    if (0 == strncmp(buffer, header, sizeof(buffer))) {
        /* new ondisk format found */
        ondisk_v1 = true;
        pan_array_size = HDSP_MAX_DEST;
        channels_per_card = HDSP_MAX_CHANNELS;
    } else {
        /* There are two different kinds of old format: pre 1.0.24 and
         * the one used for 1.0.24/1.0.24.1. We can distinguish between
         * these two by checking the file size, becase HDSP_MAX_CHANNELS
         * was bumped right before the 1.0.24 release.
         */
        fseek (file, 0, SEEK_END);
        long filesize = ftell (file);
        
        if (1163808 == filesize) {
            /* file written by hdspmixer v1.0.24 or v1.0.24.1 with
             * HDSP_MAX_CHANNELS set to 64, but pan_array_size still at
             * 14, so setting channels_per_card should get the correct
             * mapping.
             */
            channels_per_card = 64; /* HDSP_MAX_CHANNELS */
        }

        /* rewind to the start and simply read all data */
        rewind(file);
    }

    for (int speed = 0; speed < 3; ++speed) {
        for (int card = 0; card < MAX_CARDS; ++card) {
            /*!< \bug should be NUM_PRESETS or NUM_PRESETS-1 */
            for (int preset = 0; preset < 8; ++preset) {
                for (int channel = 0; channel < channels_per_card; ++channel) {
                    /* inputs pans and volumes */
                    if (fread((void *)&(inputs->strips[channel]->data[card][speed][preset]->pan_pos[0]), sizeof(int), pan_array_size, file) != pan_array_size) {
                        goto load_error;
                    }
                    if (fread((void *)&(inputs->strips[channel]->data[card][speed][preset]->fader_pos[0]), sizeof(int), pan_array_size, file) != pan_array_size) {
                        goto load_error;
                    }
                    /* playbacks pans and volumes */
                    if (fread((void *)&(playbacks->strips[channel]->data[card][speed][preset]->pan_pos[0]), sizeof(int), pan_array_size, file) != pan_array_size) {
                        goto load_error;
                    }
                    if (fread((void *)&(playbacks->strips[channel]->data[card][speed][preset]->fader_pos[0]), sizeof(int), pan_array_size, file) != pan_array_size) {
                        goto load_error;
                    }
                    /* inputs mute/solo/dest */
                    if (fread((void *)&(inputs->strips[channel]->data[card][speed][preset]->mute), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }
                    if (fread((void *)&(inputs->strips[channel]->data[card][speed][preset]->solo), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }
                    if (fread((void *)&(inputs->strips[channel]->data[card][speed][preset]->dest), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }
                    /* playbacks mute/solo/dest */
                    if (fread((void *)&(playbacks->strips[channel]->data[card][speed][preset]->mute), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }
                    if (fread((void *)&(playbacks->strips[channel]->data[card][speed][preset]->solo), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }
                    if (fread((void *)&(playbacks->strips[channel]->data[card][speed][preset]->dest), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }
                    /* outputs volumes */
                    if (fread((void *)&(outputs->strips[channel]->data[card][speed][preset]->fader_pos), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }

                }
                /* Lineouts */
                if (fread((void *)&(outputs->strips[HDSP_MAX_CHANNELS]->data[card][speed][preset]->fader_pos), sizeof(int), 1, file) != 1) {
                    goto load_error;
                }
                if (fread((void *)&(outputs->strips[HDSP_MAX_CHANNELS+1]->data[card][speed][preset]->fader_pos), sizeof(int), 1, file) != 1) {
                    goto load_error;
                }
                /* Global settings */
                if (fread((void *)&(data[card][speed][preset]->input), sizeof(int), 1, file) != 1) {
                    goto load_error;
                }
                if (fread((void *)&(data[card][speed][preset]->output), sizeof(int), 1, file) != 1) {
                    goto load_error;
                }
                if (fread((void *)&(data[card][speed][preset]->playback), sizeof(int), 1, file) != 1) {
                    goto load_error;
                }
                if (fread((void *)&(data[card][speed][preset]->submix), sizeof(int), 1, file) != 1) {
                    goto load_error;
                }
                if (fread((void *)&(data[card][speed][preset]->submix_value), sizeof(int), 1, file) != 1) {
                    goto load_error;
                }
                if (fread((void *)&(data[card][speed][preset]->solo), sizeof(int), 1, file) != 1) {
                    goto load_error;
                }
                if (fread((void *)&(data[card][speed][preset]->mute), sizeof(int), 1, file) != 1) {
                    goto load_error;
                }
                /* read additional meter settings only present in newer mix files */
                if (ondisk_v1) {
                    if (fread((void *)&(data[card][speed][preset]->last_destination), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }
                    if (fread((void *)&(data[card][speed][preset]->rmsplus3), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }
                    if (fread((void *)&(data[card][speed][preset]->numbers), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }
                    if (fread((void *)&(data[card][speed][preset]->over), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }
                    if (fread((void *)&(data[card][speed][preset]->level), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }
                    if (fread((void *)&(data[card][speed][preset]->rate), sizeof(int), 1, file) != 1) {
                        goto load_error;
                    }
                }
            }
        }
    }
    fclose(file);
    setTitleWithFilename();
    cards[current_card]->resetMixer();
    inputs->buttons->presets->preset_change(1);
    return;
load_error:
    fclose(file);
    fl_alert("Error loading presets from file %s", file_name);
    return;
}

void HDSPMixerWindow::setTitle(std::string suffix)
{
    std::string title = "HDSPMixer (";

    title = title + cards[current_card]->cardname + ") "; /*cardname */
    title = title + suffix;
    snprintf(window_title, FL_PATH_MAX, "%s", title.c_str());
    label(window_title);
}

void HDSPMixerWindow::setTitleWithFilename(void)
{
    const char *filename = fl_filename_name(file_name);

    if (NULL == file_name) {
        filename = "(unsaved)";
    }

    setTitle(filename);
}

void HDSPMixerWindow::stashPreset(void)
{
    cards[current_card]->last_preset = current_preset;
    cards[current_card]->last_dirty = dirty;
    /* save the current mixer state to the virtual 9th preset */
    inputs->buttons->presets->save_preset(9);
}

void HDSPMixerWindow::unstashPreset(void)
{
    /* load temporary data from virtual 9th preset */
    inputs->buttons->presets->restore_preset(9);
    
    current_preset = cards[current_card]->last_preset;
    /* Internal notion of playback in use. Relevant for blinking buttons */
    inputs->buttons->presets->preset = current_preset + 1;
    dirty = cards[current_card]->last_dirty;
    /* Preset masks (which preset button is green) */
    inputs->buttons->presets->presetmask = (int)pow(2, current_preset);
    if (dirty) {
        /* make the buttons blink if it's unsaved. We need to remove any
         * existing triggers to dirty_cb, because dirty_cb is called
         * every 0.3 seconds and enabling/disabling (highlight/unlight) the
         * buttons, so if we have too many callbacks, the buttons would
         * remain in one state --> no blinking. We need exactly one.
         */
        Fl::remove_timeout(dirty_cb, (void *)this);
        Fl::add_timeout(0.3, dirty_cb, (void *)this);
    } else {
        /* Hack. I don't know why this is necessary, but if we're clean,
         * we need to recall the preset again to correctly reflect
         * the dirty/undirty state.
         *
         * Though it's a little bit redundant, it at least won't do any harm.
         */
        inputs->buttons->presets->preset_change(current_preset+1);
    }
}

void HDSPMixerWindow::restoreDefaults(int card)
{
    int chnls[3];
    int maxdest[3];
    int h9632_spdif_submix[3];
    int h9632_an12_submix[3];
    int num_modes = 2;
    int ndb = inputs->strips[0]->fader->ndb;
    switch (cards[card]->type) {
    case Multiface:
        chnls[0] = 18;
        chnls[1] = 14;
        maxdest[0] = 10;
        maxdest[1] = 8;
        break;
    case Digiface:
        chnls[0] = 26;
        chnls[1] = 14;
        maxdest[0] = 14;
        maxdest[1] = 8;
        break;
    case RPM:
        chnls[0] = chnls[1] = 6;
        maxdest[0] = maxdest[1] = 3;
        break;
    case H9652:
        chnls[0] = 26;
        chnls[1] = 14;
        maxdest[0] = 13;
        maxdest[1] = 7;
        break;
    case H9632:
        chnls[0] = 16;
        chnls[1] = 12;
        chnls[2] = 8;
        maxdest[0] = 8;
        maxdest[1] = 6;
        maxdest[2] = 4;
        h9632_spdif_submix[0] = 4;
        h9632_spdif_submix[1] = 2;
        h9632_spdif_submix[2] = 0;
        h9632_an12_submix[0] = 5;
        h9632_an12_submix[1] = 3;
        h9632_an12_submix[2] = 1;
        num_modes = 3;
        break;
    case HDSPeMADI:
        chnls[0] = 64;
        chnls[1] = 32;
        chnls[2] = 16;
        maxdest[0] = 32;
        maxdest[1] = 16;
        maxdest[2] = 8;
        num_modes = 3;
        break;
    case HDSP_AES: /* these cards support full channel count at all modes */
        chnls[0] = 16;
        chnls[1] = 16;
        chnls[2] = 16;
        maxdest[0] = 8;
        maxdest[1] = 8;
        maxdest[2] = 8;
        num_modes = 3;
        break;
    case HDSPeAIO:
        chnls[0] = 14;
        chnls[1] = 10;
        chnls[2] = 8;
        maxdest[0] = 8;
        maxdest[1] = 6;
        maxdest[2] = 5;
        num_modes = 3;
        break;
    case HDSPeRayDAT:
        chnls[0] = 36;
        chnls[1] = 20;
        chnls[2] = 12;
        maxdest[0] = 18;
        maxdest[1] = 10;
        maxdest[2] = 6;
        num_modes = 3;
        break;

    default:
        /* should never happen */
        return;
    }
    /*!< \bug should be NUM_PRESETS or NUM_PRESETS-1 */
    for (int preset = 0; preset < 8; ++preset) {
        for (int speed = 0; speed < num_modes; ++speed) {
            for (int i = 0; i < 2*maxdest[speed]; i+=2) {
                for (int z = 0; z < maxdest[speed]; ++z) {
                    /* Gain setup */
                    if (cards[card]->type == H9632) {
                        inputs->strips[i]->data[card][speed][preset]->fader_pos[z] =
                                ((preset == 1 && z == h9632_an12_submix[speed]) || (i == z*2 && ((preset > 1 && preset < 4) || (preset == 7))) || ((preset == 5) && (z == h9632_spdif_submix[speed]))) ? ndb : 0;
                        inputs->strips[i+1]->data[card][speed][preset]->fader_pos[z] =
                                ((preset == 1 && z == h9632_an12_submix[speed]) || (i == z*2 && ((preset > 1 && preset < 4) || (preset == 7))) || ((preset == 5) && (z == h9632_spdif_submix[speed]))) ? ndb : 0;
                        playbacks->strips[i]->data[card][speed][preset]->fader_pos[z] =
                                ((preset == 1 && z == h9632_an12_submix[speed]) || i == z*2 || (preset == 5 && z == h9632_spdif_submix[speed])) ? ndb : 0;
                        playbacks->strips[i+1]->data[card][speed][preset]->fader_pos[z] =
                                ((preset == 1 && z == h9632_an12_submix[speed]) || i == z*2 || (preset == 5 && z == h9632_spdif_submix[speed])) ? ndb : 0;
                    } else {
                        inputs->strips[i]->data[card][speed][preset]->fader_pos[z] =
                                ((preset == 6 && z == (maxdest[speed]-1)) || (i == z*2 && (preset > 1 && preset < 4)) || (((preset > 0 && preset < 4) || preset == 7) && (z == maxdest[speed]-1))) ? ndb : 0;
                        inputs->strips[i+1]->data[card][speed][preset]->fader_pos[z] =
                                ((preset == 6 && z == (maxdest[speed]-1)) || (i == z*2 && (preset > 1 && preset < 4)) || (((preset > 0 && preset < 4) || preset == 7) && (z == maxdest[speed]-1))) ? ndb : 0;
                        playbacks->strips[i]->data[card][speed][preset]->fader_pos[z] =
                                ((preset > 4 && preset < 7 && z == (maxdest[speed]-1)) || i == z*2 || ((z == maxdest[speed]-1))) ? ndb : 0;
                        playbacks->strips[i+1]->data[card][speed][preset]->fader_pos[z] =
                                ((preset > 4 && preset < 7 && z == (maxdest[speed]-1)) || i == z*2 || ((z == maxdest[speed]-1))) ? ndb : 0;
                    }
                    /* Pan setup */
                    inputs->strips[i]->data[card][speed][preset]->pan_pos[z] = 0;
                    inputs->strips[i+1]->data[card][speed][preset]->pan_pos[z] = 28*CF;
                    playbacks->strips[i]->data[card][speed][preset]->pan_pos[z] = 0;
                    playbacks->strips[i+1]->data[card][speed][preset]->pan_pos[z] = 28*CF;
                }
                if (i < (chnls[speed]-(cards[card]->h9632_aeb.aebo ? 2 : 0))) {
                    inputs->strips[i]->data[card][speed][preset]->dest =
                            inputs->strips[i+1]->data[card][speed][preset]->dest =
                            playbacks->strips[i]->data[card][speed][preset]->dest =
                            playbacks->strips[i+1]->data[card][speed][preset]->dest = (int)floor(i/2);
                }
                outputs->strips[i]->data[card][speed][preset]->fader_pos = (preset != 4) ? 137*CF : 0;
                outputs->strips[i+1]->data[card][speed][preset]->fader_pos = (preset != 4) ? 137*CF : 0;
                if (preset == 3 || preset == 7) {
                    inputs->strips[i]->data[card][speed][preset]->mute = 1;
                    inputs->strips[i+1]->data[card][speed][preset]->mute = 1;
                    if (preset == 7) {
                        playbacks->strips[i]->data[card][speed][preset]->mute = 1;
                        playbacks->strips[i+1]->data[card][speed][preset]->mute = 1;
                    }
                }
            }
            if (cards[card]->type == H9632) {
                if (preset == 1 || preset == 6) {
                    data[card][speed][preset]->submix_value = h9632_an12_submix[speed];
                    outputs->strips[h9632_an12_submix[speed]*2]->data[card][speed][preset]->fader_pos = ndb;
                    outputs->strips[h9632_an12_submix[speed]*2+1]->data[card][speed][preset]->fader_pos = ndb;
                } else if (preset == 5) {
                    data[card][speed][preset]->submix_value = h9632_spdif_submix[speed];
                    outputs->strips[h9632_spdif_submix[speed]*2]->data[card][speed][preset]->fader_pos = ndb;
                    outputs->strips[h9632_spdif_submix[speed]*2+1]->data[card][speed][preset]->fader_pos = ndb;
                } else {
                    data[card][speed][preset]->submix = 0;
                }
            } else if (preset > 4 && preset < 7) {
                data[card][speed][preset]->submix_value = maxdest[speed]-1;
                if (preset == 5) {
                    outputs->strips[chnls[speed]-2]->data[card][speed][preset]->fader_pos = ndb;
                    outputs->strips[chnls[speed]-1]->data[card][speed][preset]->fader_pos = ndb;
                }
            } else {
                data[card][speed][preset]->submix = 0;
            }
            if (preset == 3 || preset == 7) {
                data[card][speed][preset]->mute = 1;
            }
        }
    }
}

HDSPMixerWindow::HDSPMixerWindow(int x, int y, int w, int h, const char *label, HDSPMixerCard *hdsp_card1, HDSPMixerCard *hdsp_card2, HDSPMixerCard *hdsp_card3):Fl_Double_Window(x, y, w, h, label)
{
    int i;
    cards[0] = hdsp_card1;
    cards[1] = hdsp_card2;
    cards[2] = hdsp_card3;
    current_card = current_preset = 0;
    prefs = new Fl_Preferences(Fl_Preferences::USER, "thomasATundata.org", "HDSPMixer");
    if (!prefs->get("default_file", file_name_buffer, NULL, FL_PATH_MAX-1)) {
        file_name = NULL;
    } else {
        struct stat buf;
        if (!stat(file_name_buffer, &buf)) {
            file_name = file_name_buffer;
        } else {
            file_name = NULL;
            prefs->deleteEntry("default_file");
            prefs->flush();
        }
    }
    for (int j = 0; j < MAX_CARDS; j++) {
        for (int i = 0; i < NUM_PRESETS; ++i) {
            data[j][0][i] = new HDSPMixerPresetData();
            data[j][1][i] = new HDSPMixerPresetData();
            data[j][2][i] = new HDSPMixerPresetData();
        }
    }
    buttons_removed = 0;
    dirty = 0;
    scroll = new Fl_Scroll(0, 0, w, h);
    menubar = new Fl_Menu_Bar(0, 0, w, MENU_HEIGHT);
    menubar->textfont(FL_HELVETICA);
    menubar->textsize(12);
    menubar->box(FL_THIN_UP_BOX);
    menubar->add("&File/Open preset file", FL_CTRL+'o', (Fl_Callback *)open_cb, (void *)this);
    menubar->add("&File/Save preset file", FL_CTRL+'s', (Fl_Callback *)save_cb, (void *)this);
    menubar->add("&File/Save preset file as ...", 0, (Fl_Callback *)save_as_cb, (void *)this, FL_MENU_DIVIDER);
    menubar->add("&File/Make current file default", 'd', (Fl_Callback *)make_default_cb, (void *)this);
    menubar->add("&File/Restore factory settings", 'f', (Fl_Callback *)restore_defaults_cb, (void *)this, FL_MENU_DIVIDER);
    menubar->add("&File/E&xit", FL_CTRL+'q', (Fl_Callback *)exit_cb, (void *)this);
    menubar->add("&View/Input", 'i', (Fl_Callback *)view_cb, (void *)this, FL_MENU_TOGGLE|FL_MENU_VALUE);
    menubar->add("&View/Playback", 'p', (Fl_Callback *)view_cb, (void *)this, FL_MENU_TOGGLE|FL_MENU_VALUE);
    menubar->add("&View/Output", 'o', (Fl_Callback *)view_cb, (void *)this, FL_MENU_DIVIDER|FL_MENU_TOGGLE|FL_MENU_VALUE);
    menubar->add("&View/Submix", 's', (Fl_Callback *)submix_cb, (void *)this, FL_MENU_TOGGLE|FL_MENU_VALUE);
    menubar->add("&Options/Level Meter Setup", 'm', (Fl_Callback *)setup_cb, (void *)this);
    menubar->add("&?/About", 0, (Fl_Callback *)about_cb, (void *)this);
    inputs = new HDSPMixerInputs(0, MENU_HEIGHT, w, FULLSTRIP_HEIGHT, cards[0]->channels_input);
    inputs->buttons->input = 1;
    inputs->buttons->output = 1;
    inputs->buttons->submix = 1;
    inputs->buttons->playback = 1;
    playbacks = new HDSPMixerPlaybacks(0, MENU_HEIGHT+FULLSTRIP_HEIGHT, w, FULLSTRIP_HEIGHT, cards[0]->channels_playback);
    outputs = new HDSPMixerOutputs(0, MENU_HEIGHT+FULLSTRIP_HEIGHT*2, w, SMALLSTRIP_HEIGHT, cards[0]->channels_output);
    scroll->end();
    end();
    setup = new HDSPMixerSetup(400, 260, "Level Meters Setup", this);
    about = new HDSPMixerAbout(360, 300, "About HDSPMixer", this);
    i = 0;
    while (i < MAX_CARDS && cards[i] != NULL) {
        cards[i++]->initializeCard(this);
        actualizeStrips();
    }
    size_range(MIN_WIDTH, MIN_HEIGHT, (cards[current_card]->channels_playback+2)*STRIP_WIDTH, FULLSTRIP_HEIGHT*2+SMALLSTRIP_HEIGHT+MENU_HEIGHT);
    cards[current_card]->resetMixer();
    if (file_name) {
        printf("Restoring last presets used\n");
        load();
    } else {
        printf("Initializing default presets\n");
        i = 0;
        while (i < MAX_CARDS && cards[i] != NULL) {
            current_card = i;
            restoreDefaults(i++);
            inputs->buttons->presets->preset_change(1);
        }
    }
    Fl::atclose = atclose_cb;
    Fl::add_handler(handler_cb);
    Fl::add_timeout(0.030, readregisters_cb, this);
    i = 0;
    while (i < MAX_CARDS && cards[i] != NULL) {
        current_card = i;
        inputs->buttons->cardselector->ActivateCard (i++);
    }
}

int HDSPMixerWindow::handle(int e) 
{
    return Fl_Double_Window::handle(e);
}

void HDSPMixerWindow::resize(int x, int y, int w, int h)
{
    Fl_Double_Window::resize(x, y, w, h);
    scroll->resize (0, 0, w, h);
}

/*! reorder after changing which strips are shown
 *
 */
void HDSPMixerWindow::reorder()
{
    int xpos = scroll->x();
    int ypos = scroll->y();
    int ytemp = ypos+MENU_HEIGHT;
    if (inputs->buttons->view->input) {
        scroll->add(inputs);
        inputs->add(*(inputs->buttons));
        buttons_removed = 0;
        inputs->buttons->position(inputs->buttons->x(), MENU_HEIGHT);
        inputs->position(xpos, ytemp);
        ytemp += FULLSTRIP_HEIGHT;
    } else {
        if (!buttons_removed) {
            buttons_removed = 1;
            playbacks->add(*(inputs->buttons));
            inputs->buttons->position(playbacks->empty->x(), playbacks->empty->y());
        }
        scroll->remove(*inputs);
    }
    if (inputs->buttons->view->playback) {
        scroll->add(playbacks);
        playbacks->position(xpos, ytemp);
        ytemp += FULLSTRIP_HEIGHT;
    } else {
        scroll->remove(*playbacks);
    }
    if (inputs->buttons->view->output) {
        scroll->add(outputs);
        outputs->position(xpos, ytemp);
        ytemp += SMALLSTRIP_HEIGHT;
    } else {
        scroll->remove(*outputs);
    }
    scroll->init_sizes();
    resize(x(), y(), w(), ytemp);
    size_range(MIN_WIDTH, MIN_HEIGHT, (cards[current_card]->channels_playback+2)*STRIP_WIDTH, ytemp);
}

void HDSPMixerWindow::checkState()
{
    int speed = cards[current_card]->speed_mode;
    int p = inputs->buttons->presets->preset-1;
    int corrupt = 0;
    /* Mixer strips */
    for (int i = 0; i < HDSP_MAX_CHANNELS; ++i) {
        for (int j = 0; j < HDSP_MAX_DEST; ++j) {
            /* Inputs */
            if (inputs->strips[i]->data[current_card][speed][p]->pan_pos[j] != inputs->strips[i]->pan->pos[j])
                corrupt++;
            if (inputs->strips[i]->data[current_card][speed][p]->fader_pos[j] != inputs->strips[i]->fader->pos[j])
                corrupt++;
            if (playbacks->strips[i]->data[current_card][speed][p]->pan_pos[j] != playbacks->strips[i]->pan->pos[j])
                corrupt++;
            if (playbacks->strips[i]->data[current_card][speed][p]->fader_pos[j] != playbacks->strips[i]->fader->pos[j])
                corrupt++;
        }
        /* Inputs row */
        if (inputs->strips[i]->data[current_card][speed][p]->mute != inputs->strips[i]->mutesolo->mute)
            corrupt++;
        if (inputs->strips[i]->data[current_card][speed][p]->solo != inputs->strips[i]->mutesolo->solo)
            corrupt++;
        if (inputs->strips[i]->data[current_card][speed][p]->dest != inputs->strips[i]->targets->selected)
            corrupt++;
        /* Playbacks row */
        if (playbacks->strips[i]->data[current_card][speed][p]->mute != playbacks->strips[i]->mutesolo->mute)
            corrupt++;
        if (playbacks->strips[i]->data[current_card][speed][p]->solo != playbacks->strips[i]->mutesolo->solo)
            corrupt++;
        if (playbacks->strips[i]->data[current_card][speed][p]->dest != playbacks->strips[i]->targets->selected)
            corrupt++;
        /* Outputs row */
        if (outputs->strips[i]->data[current_card][speed][p]->fader_pos != outputs->strips[i]->fader->pos[0])
            corrupt++;
    }

    /* Global settings */
    if (data[current_card][speed][p]->mute != inputs->buttons->master->mute)
        corrupt++;
    if (data[current_card][speed][p]->solo != inputs->buttons->master->solo)
        corrupt++;
    if (data[current_card][speed][p]->input != inputs->buttons->view->input)
        corrupt++;
    if (data[current_card][speed][p]->output != inputs->buttons->view->output)
        corrupt++;
    if (data[current_card][speed][p]->playback != inputs->buttons->view->playback)
        corrupt++;
    if (data[current_card][speed][p]->submix != inputs->buttons->view->submix)
        corrupt++;
    if (data[current_card][speed][p]->submix_value != inputs->buttons->view->submix_value)
        corrupt++;
    /* Setup options */
    if (setup->over_val != data[current_card][speed][p]->over)
        corrupt++;
    if (setup->rate_val != data[current_card][speed][p]->rate)
        corrupt++;
    if (setup->level_val != data[current_card][speed][p]->level)
        corrupt++;
    if (setup->rmsplus3_val != data[current_card][speed][p]->rmsplus3)
        corrupt++;
    if (setup->numbers_val != data[current_card][speed][p]->numbers)
        corrupt++;

    if (corrupt) {
        if (!dirty) {
            dirty = 1;
            setTitleWithFilename();
            Fl::add_timeout(0.3, dirty_cb, (void *)this);
        }
    } else {
        setTitleWithFilename();
        dirty = 0;
    }
}

void HDSPMixerWindow::setSubmix(int submix_value)
{
    for (int i = 0; i < cards[current_card]->channels_playback; i++) {
        inputs->strips[i]->targets->value(submix_value);
        inputs->strips[i]->targets->redraw();
        inputs->strips[i]->fader->dest = submix_value;
        inputs->strips[i]->fader->redraw();
        inputs->strips[i]->pan->dest = submix_value;
        inputs->strips[i]->pan->redraw();
        inputs->strips[i]->fader->sendGain();
        playbacks->strips[i]->targets->value(submix_value);
        playbacks->strips[i]->targets->redraw();
        playbacks->strips[i]->fader->dest = submix_value;
        playbacks->strips[i]->fader->redraw();
        playbacks->strips[i]->pan->dest = submix_value;
        playbacks->strips[i]->pan->redraw();
        playbacks->strips[i]->fader->sendGain();
    }
}

void HDSPMixerWindow::unsetSubmix()
{
    for (int i = 0; i < cards[current_card]->channels_input; i++) {
        inputs->strips[i]->targets->value(inputs->strips[i]->targets->selected);
        inputs->strips[i]->targets->redraw();
        inputs->strips[i]->fader->dest = inputs->strips[i]->targets->selected;
        inputs->strips[i]->fader->redraw();
        inputs->strips[i]->pan->dest = inputs->strips[i]->targets->selected;
        inputs->strips[i]->pan->redraw();
        inputs->strips[i]->fader->sendGain();
        playbacks->strips[i]->targets->value(playbacks->strips[i]->targets->selected);
        playbacks->strips[i]->targets->redraw();
        playbacks->strips[i]->fader->dest = playbacks->strips[i]->targets->selected;
        playbacks->strips[i]->fader->redraw();
        playbacks->strips[i]->pan->dest = playbacks->strips[i]->targets->selected;
        playbacks->strips[i]->pan->redraw();
        playbacks->strips[i]->fader->sendGain();
    }
}


void HDSPMixerWindow::refreshMixer()
{
    int i, j;
    for (i = 1; i <= cards[current_card]->channels_input; ++i) {
        for (j = 0; j < inputs->strips[0]->targets->max_dest; ++j) {
            setMixer(i, 0, j);
            setMixer(i, 1, j);
        }
    }
}

void HDSPMixerWindow::refreshMixerStrip(int idx, int src)
{
    int i;
    for (i = 0; i < inputs->strips[0]->targets->max_dest; ++i) {
        setMixer(idx, src, i);
    }
}

void HDSPMixerWindow::setMixer(int idx, int src, int dst)
{
    /*  idx is the strip number (indexed fom 1)
    src is the row (0 = inputs, 1 = playbacks, 2 = outputs)
    dst is the destination stereo channel
    */
    int err,gsolo_active,gmute_active, gmute, gsolo;

    gsolo_active = inputs->buttons->master->solo_active;
    gmute_active = inputs->buttons->master->mute_active;
    gsolo = inputs->buttons->master->solo;
    gmute = inputs->buttons->master->mute;
    
    if (src == 0 || src == 1) {

        double vol, pan, attenuation_l, attenuation_r, left_val, right_val;

        if (src) {
            if ((gmute && playbacks->strips[idx-1]->mutesolo->mute && !(playbacks->strips[idx-1]->mutesolo->solo && gsolo)) || (gsolo && gsolo_active && !(playbacks->strips[idx-1]->mutesolo->solo)) ) {
                left_val = right_val = 0;
                goto muted;
            }
        } else {
            if ((gmute && inputs->strips[idx-1]->mutesolo->mute && !(inputs->strips[idx-1]->mutesolo->solo && gsolo)) || (gsolo && gsolo_active && !(inputs->strips[idx-1]->mutesolo->solo)) ) {
                left_val = right_val = 0;
                goto muted;
            }
        }

        if (src) {
            vol = playbacks->strips[idx-1]->fader->posToInt(playbacks->strips[idx-1]->fader->pos[dst]);
            pan = (double)(playbacks->strips[idx-1]->pan->pos[dst])/(double)(PAN_WIDTH*CF);
        } else {
            vol = inputs->strips[idx-1]->fader->posToInt(inputs->strips[idx-1]->fader->pos[dst]);
            pan = (double)(inputs->strips[idx-1]->pan->pos[dst])/(double)(PAN_WIDTH*CF);
        }
        attenuation_l = (double)(outputs->strips[dst*2]->fader->posToInt(outputs->strips[dst*2]->fader->pos[0]))/65535.0;
        attenuation_r = (double)(outputs->strips[dst*2+1]->fader->posToInt(outputs->strips[dst*2+1]->fader->pos[0]))/65535.0;

        left_val = attenuation_l* vol * (1.0 - pan);
        right_val = attenuation_r* vol * pan;

muted:
        if (src == 0 ){
            cards[current_card]->setInput(idx-1,dst,left_val,right_val);
        } else {
            cards[current_card]->setPlayback(idx-1,dst,left_val,right_val);
        }

    } else if (src == 2) {
        int i, vol, dest;

        dest = (int)floor((idx-1)/2);

        for (i = 0; i < cards[current_card]->channels_input; ++i) {
            if ((vol = inputs->strips[i]->fader->posToInt(inputs->strips[i]->fader->pos[dest])) != 0) {
                setMixer(i+1, 0, dest);
            }
        }

        for (i = 0; i < cards[current_card]->channels_playback; ++i) {
            if ((vol = playbacks->strips[i]->fader->posToInt(playbacks->strips[i]->fader->pos[dest])) != 0) {
                setMixer(i+1, 1, dest);
            }
        }
    }
}

void HDSPMixerWindow::actualizeStrips()
{
    for (int i = 0; i < HDSP_MAX_CHANNELS; ++i) {
        if (i < cards[current_card]->channels_input) {
            inputs->strips[i]->show();
        } else {
            inputs->strips[i]->hide();
        }

        if (i < cards[current_card]->channels_playback) {
            playbacks->strips[i]->show();
        } else {
            playbacks->strips[i]->hide();
        }

        if (i < cards[current_card]->channels_output) {
            outputs->strips[i]->show();
        } else {
            outputs->strips[i]->hide();
        }
    }
    if (cards[current_card]->h9632_aeb.aebi && !cards[current_card]->h9632_aeb.aebo) {
        for (int i = 0; i < 2; ++i) {
            inputs->empty_aebi[i]->hide();
            playbacks->empty_aebo[i]->show();
            outputs->empty_aebo[i]->show();
        }
        for (int i = cards[current_card]->channels_playback-4; i < cards[current_card]->channels_playback; ++i) {
            playbacks->strips[i]->hide();
            outputs->strips[i]->hide();
        }
    } else if (cards[current_card]->h9632_aeb.aebo && !cards[current_card]->h9632_aeb.aebi) {
        for (int i = 0; i < 2; ++i) {
            inputs->empty_aebi[i]->show();
            playbacks->empty_aebo[i]->hide();
            outputs->empty_aebo[i]->hide();
        }
        for (int i = cards[current_card]->channels_input-4; i < cards[current_card]->channels_input; ++i) {
            inputs->strips[i]->hide();
        }
    } else {
        for (int i = 0; i < 2; ++i) {
            inputs->empty_aebi[i]->hide();
            playbacks->empty_aebo[i]->hide();
            outputs->empty_aebo[i]->hide();
        }
    }
    if (cards[current_card]->type != H9652 && cards[current_card]->type != H9632) outputs->empty->hide();
}

void HDSPMixerWindow::updateMode(){
    HDSPMixerCard *card = cards[current_card];

    actualizeStrips();

    for (int i = 0; i < card->channels_input; ++i) {
        inputs->strips[i]->targets->setLabels();
    }
    for (int i = 0; i < card->channels_playback; ++i) {
        playbacks->strips[i]->targets->setLabels();
    }
    for (int i = 0; i < card->channels_output; ++i) {
        outputs->strips[i]->setLabels();
    }

    if (card->h9632_aeb.aebo && !card->h9632_aeb.aebi) {
        inputs->empty_aebi[0]->position(STRIP_WIDTH*(card->channels_input-4), inputs->empty_aebi[0]->y());
        inputs->empty_aebi[1]->position(STRIP_WIDTH*(card->channels_input-2), inputs->empty_aebi[1]->y());
    } else if (card->h9632_aeb.aebi && !card->h9632_aeb.aebo) {
        playbacks->empty_aebo[0]->position(STRIP_WIDTH*(card->channels_playback-4), playbacks->empty_aebo[0]->y());
        playbacks->empty_aebo[1]->position(STRIP_WIDTH*(card->channels_playback-2), playbacks->empty_aebo[1]->y());
        outputs->empty_aebo[0]->position(STRIP_WIDTH*(card->channels_playback-4), outputs->empty_aebo[0]->y());
        outputs->empty_aebo[1]->position(STRIP_WIDTH*(card->channels_playback-2), outputs->empty_aebo[1]->y());
    }
    inputs->buttons->position(STRIP_WIDTH*card->channels_input, inputs->buttons->y());
    inputs->init_sizes();
    playbacks->empty->position(STRIP_WIDTH*card->channels_playback, playbacks->empty->y());
    playbacks->init_sizes();
    outputs->empty->position(STRIP_WIDTH*(card->channels_playback), outputs->empty->y());
    outputs->init_sizes();
    int window_width = (card->channels_playback+2)*STRIP_WIDTH;
    int window_height = FULLSTRIP_HEIGHT*2+SMALLSTRIP_HEIGHT+MENU_HEIGHT;
    inputs->size(window_width, inputs->h());
    playbacks->size(window_width, playbacks->h());
    outputs->size(window_width, outputs->h());
    scroll->init_sizes();
    ((Fl_Widget *)(menubar))->size(window_width, menubar->h());
    size_range(MIN_WIDTH, MIN_HEIGHT, window_width, window_height);
    resize(x(), y(), window_width, h());
    reorder();

    inputs->buttons->presets->preset_change(1);
}


