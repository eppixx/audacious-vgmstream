#include <glib.h>
#include <unistd.h>
#include <stdlib.h>

#include <audacious/input.h>
#include <audacious/plugin.h>
#include <audacious/i18n.h>

#include "../src/vgmstream.h"
#include "main.h"
#include "version.h"
#include "vfs.h"
#include "settings.h"

VGMSTREAM *vgmstream = NULL;
gint stream_samples_amount;

static void close_stream();

//this method represents the core loop and runs in it's own thread
//it is manipulatable through flags
static void vgmstream_play_loop(void)
{
  debugMessage("start loop");

  //local variables
  gshort  buffer[576*vgmstream->channels];
  gint    seek_needed_samples;
  gint    samples_to_do;
  gint    current_sample_pos = 0;
  gint    fade_sample_length;

  gint max_buffer_samples = sizeof(buffer)/sizeof(buffer[0])/vgmstream->channels;
  if (vgmstream->loop_flag)
    fade_sample_length = vgmstream_cfg.fade_length * vgmstream->sample_rate;
  else
    fade_sample_length = -1;

  while (!aud_input_check_stop())
  {
      /******************************************
                        Seeking
       ******************************************/
    int decode_seek = aud_input_check_seek();

    if (decode_seek >= 0)
    {
      // compute from ms to samples
      seek_needed_samples = (long long)decode_seek * vgmstream->sample_rate / 1000L;
      if (seek_needed_samples < current_sample_pos)
      {
        // go back in time, reopen file
        debugMessage("reopen file to seek backward");
        reset_vgmstream(vgmstream);
        current_sample_pos = 0;
        samples_to_do = seek_needed_samples;
      }
      else if (current_sample_pos < seek_needed_samples)
      {
        // go forward in time
        samples_to_do = seek_needed_samples - current_sample_pos;
      }
      else
      {
        // seek to where we are, how convenient
        samples_to_do = -1;
      }

      // do the actual seeking
      if (samples_to_do >= 0)
      {
        debugMessage("render forward");
        
        //render till seeked sample
        while (samples_to_do >0)
        {
          gint seek_samples = max_buffer_samples;
          current_sample_pos += seek_samples;
          samples_to_do -= seek_samples;
          render_vgmstream(buffer, seek_samples, vgmstream);
        }
        debugMessage("after render vgmstream");

        //reset variables
        samples_to_do = -1;
      }
    }

      /******************************************
                        Playback
       ******************************************/
      // read data and pass onward
      samples_to_do = MIN(max_buffer_samples, stream_samples_amount - (current_sample_pos + max_buffer_samples));
      if (!(samples_to_do) || !(vgmstream->channels))
        break;
      else
      {
        // render audio data and write to buffer
        render_vgmstream(buffer,samples_to_do,vgmstream);

        // fade!
        if (vgmstream->loop_flag && fade_sample_length > 0 && !vgmstream_cfg.loop_forever) 
        {
            gint samples_into_fade = current_sample_pos - (stream_samples_amount - fade_sample_length);
            if (samples_into_fade + samples_to_do > 0) 
            {
              debugMessage("start fading");
              gint j,k;
              for (j=0; j < samples_to_do; ++j, ++samples_into_fade) 
              {
                if (samples_into_fade > 0) 
                {
                  gdouble fadedness = (gdouble)(fade_sample_length-samples_into_fade)/fade_sample_length;
                  for (k=0; k < vgmstream->channels; ++k) 
                  {
                      buffer[j*vgmstream->channels+k] = (gshort)(buffer[j*vgmstream->channels+k]*fadedness);
                  }
                }
              }
            }
          }

        // pass it on
        aud_input_write_audio(buffer, sizeof(buffer));
        current_sample_pos += samples_to_do;
      }
  }
  debugMessage("track ending");

  current_sample_pos = 0;
  close_stream();
}

gboolean vgmstream_play(const gchar *filename, VFSFile *file)
{
  debugMessage("start play");
  STREAMFILE *streamfile = open_vfs(filename);
  vgmstream = init_vgmstream_from_STREAMFILE(streamfile);
  close_streamfile(streamfile);
  
  if (!vgmstream || vgmstream->channels <= 0)  
  {
    printf("Error::Channels are zero or couldn't init plugin\n");
    close_stream();
    return FALSE;
  }

  //FMT_S16_LE is the simple wav-format
  if (!aud_input_open_audio(FMT_S16_LE, vgmstream->sample_rate, vgmstream->channels))
  {
    printf("Error::Couldn't open audio device\n");
    close_stream();
    return FALSE;
  }

  stream_samples_amount = get_vgmstream_play_samples(vgmstream_cfg.loop_count,vgmstream_cfg.fade_length,vgmstream_cfg.fade_delay,vgmstream);
  gint ms   = (stream_samples_amount * 1000LL) / vgmstream->sample_rate;
  gint rate = vgmstream->sample_rate * 2 * vgmstream->channels;

  //set Tuple for track info
  //if loop_forever don't set FIELD_LENGTH
  Tuple * tuple = tuple_new_from_filename(filename);
  tuple_set_int(tuple, FIELD_BITRATE, rate);
  if (!vgmstream_cfg.loop_forever)
    tuple_set_int(tuple, FIELD_LENGTH, ms);
  aud_input_set_tuple(tuple);

  //start play_loop
  vgmstream_play_loop();

  return TRUE;
}

gboolean vgmstream_init()
{
  debugMessage("init");
  vgmstream_cfg_load();
  debugMessage("after init");
  return TRUE;
}

static void close_stream()
{
  debugMessage("called close_stream");
  if (vgmstream)
    close_vgmstream(vgmstream);
  vgmstream = NULL;
}

// called every time the user adds a new file to playlist
Tuple* vgmstream_probe_for_tuple(const gchar *filename, VFSFile *file)
{
  debugMessage("probe for tuple");
  Tuple       *tuple      = NULL;
  gint         ms;
  gint         rate;
  VGMSTREAM   *vgmstream  = NULL;
  STREAMFILE  *streamfile = NULL;

  streamfile = open_vfs(filename);
  vgmstream  = init_vgmstream_from_STREAMFILE(streamfile);

  tuple = tuple_new_from_filename(filename);
  rate  = vgmstream->sample_rate * 2 * vgmstream->channels;
  tuple_set_int(tuple, FIELD_BITRATE, rate);
  
  //if loop_forever return tuple with empty FIELD_LENGTH
  if (!vgmstream_cfg.loop_forever)
  {
    ms = get_vgmstream_play_samples(vgmstream_cfg.loop_count,vgmstream_cfg.fade_length,vgmstream_cfg.fade_delay,vgmstream) 
      * 1000LL / vgmstream->sample_rate;
    tuple_set_int(tuple, FIELD_LENGTH, ms);
  }

  close_streamfile(streamfile);
  close_vgmstream(vgmstream);
  return tuple;
}

