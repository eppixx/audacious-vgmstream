#include <glib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include <audacious/plugin.h>
#include <audacious/i18n.h>

#include "../src/vgmstream.h"
#include "version.h"
#include "vfs.h"
#include "settings.h"

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


VGMSTREAM *vgmstream = NULL;
volatile glong decode_seek;
gint stream_samples_amount;

static void close_stream();

//threads
GCond *ctrl_cond = NULL;
GMutex *ctrl_mutex = NULL;

gboolean playing;
gboolean eof;
gboolean end_thread;

//this method represents the core loop and runs in it's own thread
static void* vgmstream_play_loop(InputPlayback *playback)
{
  debugMessage("start loop");

  //local variables
  gshort buffer[576*vgmstream->channels];
  glong l;
  gint seek_needed_samples;
  gint samples_to_do;
  gint current_sample_pos = 0;
  gint fade_sample_length;

  //init loop variables
  decode_seek = -1;
  playing = TRUE;
  eof = FALSE;
  end_thread = FALSE;
        
  gint max_buffer_samples = sizeof(buffer)/sizeof(buffer[0])/vgmstream->channels;
  if (vgmstream->loop_flag)
    fade_sample_length = vgmstream_cfg.fade_length * vgmstream->sample_rate;
  else
    fade_sample_length = -1;

  while (playing)
  {
      /******************************************
                        Seeking
       ******************************************/

    // check thread flags, not my favorite method
    if (end_thread)
    {
      goto exit_thread;
    }
    else if (decode_seek >= 0)
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
          // printf("%d\n", samples_to_do);
          render_vgmstream(buffer, seek_samples, vgmstream);
        }

        debugMessage("after render vgmstream");
        
        playback->output->flush(decode_seek);

        //reset variables
        samples_to_do = -1;
        eof = FALSE;
      }
      decode_seek = -1;
    }

      /******************************************
                        Playback
       ******************************************/
    if (!eof)
    {
      // read data and pass onward
      samples_to_do = min(max_buffer_samples, stream_samples_amount - (current_sample_pos + max_buffer_samples));
      l = (samples_to_do * vgmstream->channels*2);
      if (!l)
      {
        debugMessage("set eof flag");
        eof = TRUE;
        // will trigger on next run through
      }
      else
      {
        // ok we read stuff
        render_vgmstream(buffer,samples_to_do,vgmstream);

        // fade!
        if (vgmstream->loop_flag && fade_sample_length > 0 && !vgmstream_cfg.loop_forever) 
        {
            debugMessage("start fading");
            gint samples_into_fade = current_sample_pos - (stream_samples_amount - fade_sample_length);
            if (samples_into_fade + samples_to_do > 0) {
                gint j,k;
                for (j=0;j<samples_to_do;j++,samples_into_fade++) {
                    if (samples_into_fade > 0) {
                        gdouble fadedness = (gdouble)(fade_sample_length-samples_into_fade)/fade_sample_length;
                        for (k=0;k<vgmstream->channels;k++) {
                            buffer[j*vgmstream->channels+k] =
                                (gshort)(buffer[j*vgmstream->channels+k]*fadedness);
                        }
                    }
                }
            }
          }

          // pass it on
        // playback->pass_audio(playback,FMT_S16_LE,vgmstream->channels , l , buffer , playing );
        playback->output->write_audio(buffer, sizeof(buffer));
        current_sample_pos += samples_to_do;
      }
    }
    else
    {
      debugMessage("waiting for track ending");
      // at EOF
      while (playback->output->buffer_playing())
        g_usleep(10000);

      playing = FALSE;
      // this effectively ends the loop
    }
  }
  debugMessage("track ending");
  playback->output->close_audio();

 exit_thread:
  decode_seek = -1;
  playing = FALSE;
  current_sample_pos = 0;
  close_stream();

  return 0;
}

void vgmstream_play(InputPlayback *playback, const char *filename, 
  VFSFile * file, int start_time, int stop_time, bool_t pause)
{
  debugMessage("start play");
  vgmstream = init_vgmstream_from_STREAMFILE(open_vfs(filename));

  if (!vgmstream || vgmstream->channels <= 0)  
  {
    printf("Channels are zero or couldn't init plugin\n");
    close_stream();
    goto end_thread;
  }

  if (!playback->output->open_audio(FMT_S16_LE, vgmstream->sample_rate, vgmstream->channels))
  {
    printf("couldn't open audio device\n");
    close_stream();
    goto end_thread;
  }

  stream_samples_amount = get_vgmstream_play_samples(vgmstream_cfg.loop_count,vgmstream_cfg.fade_length,vgmstream_cfg.fade_delay,vgmstream);
  printf("%i\n", stream_samples_amount);
  gint ms = (stream_samples_amount * 1000LL) / vgmstream->sample_rate;
  gint rate = vgmstream->sample_rate * 2 * vgmstream->channels;

  //set Tuple for track info
  Tuple * tuple = tuple_new_from_filename(filename);
  tuple_set_int(tuple, FIELD_LENGTH, NULL, ms);
  tuple_set_int(tuple, FIELD_BITRATE, NULL, rate);
  //tuple_set_str(tuple,FIELD_QUALITY , NULL, "lossless"); //eigen ?erweitern?
  playback->set_tuple(playback, tuple);

  playback->set_pb_ready(playback);
  vgmstream_play_loop(playback);

end_thread:
  g_mutex_lock(ctrl_mutex);
  g_cond_signal(ctrl_cond);
  g_mutex_unlock(ctrl_mutex);
}

void vgmstream_init()
{
  debugMessage("init threads");
  vgmstream_cfg_load();
  ctrl_cond = g_cond_new();
  ctrl_mutex = g_mutex_new();
  debugMessage("after init threads");
}

void vgmstream_destroy()
{
  debugMessage("destroy threads");
  g_cond_free(ctrl_cond);
  ctrl_cond = NULL;
  g_mutex_free(ctrl_mutex);
  ctrl_mutex = NULL;
}

static void close_stream()
{
  debugMessage("called close_stream");
  if (vgmstream)
    close_vgmstream(vgmstream);
  vgmstream = NULL;
}

void vgmstream_stop(InputPlayback *playback)
{
  debugMessage("stop");
  if (vgmstream)
  {
    // kill thread
    end_thread = TRUE;
    // wait for it to die
    g_mutex_lock(ctrl_mutex);
    g_cond_wait(ctrl_cond, ctrl_mutex);
    g_mutex_unlock(ctrl_mutex);
  }
  // close audio output
  playback->output->close_audio();
  // cleanup
  // close_stream();
}

void vgmstream_pause(InputPlayback *playback, gshort paused)
{
  debugMessage("pause");
  playback->output->pause(paused);
}

void vgmstream_mseek(InputPlayback *playback, gint ms)
{
  debugMessage("mseek");
  if (vgmstream)
  {
    decode_seek = ms;
    eof = FALSE;

    //stall while vgmstream_play_loop seeks
    while (decode_seek != -1)
      g_usleep(10000);
  }
}

// called every time the user adds a new file to playlist
Tuple* vgmstream_probe_for_tuple(const gchar *filename, VFSFile *file)
{
  debugMessage("probe for tuple");
  Tuple       *tuple;
  glong        ms;
  VGMSTREAM   *vgm;

  vgm = init_vgmstream_from_STREAMFILE(open_vfs(filename));
  tuple = tuple_new_from_filename(filename);
  ms = get_vgmstream_play_samples(vgmstream_cfg.loop_count,vgmstream_cfg.fade_length,vgmstream_cfg.fade_delay,vgm) * 1000LL / vgm->sample_rate;
  
  tuple_set_int(tuple, FIELD_LENGTH, NULL, ms);
  printf("%i\n", ms);
  return tuple;
}


/*

TODO:
  free string warning
  get_vgmstream_play_samples returns strange value
  

*/
