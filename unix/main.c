#include <glib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include <audacious/plugin.h>
#include <audacious/i18n.h>

#include "version.h"
#include "../src/vgmstream.h"
#include "gui.h"
#include "vfs.h"
#include "settings.h"

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


VGMSTREAM *vgmstream = NULL;
volatile long decode_seek;
gint stream_length_samples;

//threads
GCond *ctrl_cond = NULL;
GMutex *ctrl_mutex = NULL;

//***testing***
bool_t playing;
bool_t eof;
bool_t end_thread;



void CLOSE_STREAM()
{
  if (vgmstream)
    close_vgmstream(vgmstream);
  vgmstream = NULL;
}

void* vgmstream_play_loop(InputPlayback *playback)
{
  debugMessage("start loop");

  //local variables
  int16_t buffer[576*vgmstream->channels];
  long l;
  gint seek_needed_samples;
  gint samples_to_do;
  gint decode_pos_samples = 0;
  gint fade_length_samples;

  //define other variables
  decode_seek = -1;
  playing = TRUE;
  eof = FALSE;
  end_thread = FALSE;

  if (vgmstream->loop_flag)
    fade_length_samples = FADESECONDS * vgmstream->sample_rate;
  else
    fade_length_samples = -1;

  while (playing)
  {
    // ******************************************
    // Seeking
    // ******************************************
    // check thread flags, not my favorite method
    if (end_thread)
    {
      goto exit_thread;
    }
    else if (decode_seek >= 0)
    {
      /* compute from ms to samples */
      seek_needed_samples = (long long)decode_seek * vgmstream->sample_rate / 1000L;
      if (seek_needed_samples < decode_pos_samples)
      {
        /* go back in time, reopen file */
        debugMessage("reopen file to seek backward");
        reset_vgmstream(vgmstream);
        decode_pos_samples = 0;
        samples_to_do = seek_needed_samples;
      }
      else if (decode_pos_samples < seek_needed_samples)
      {
        /* go forward in time */
        samples_to_do = seek_needed_samples - decode_pos_samples;
      }
      else
      {
        /* seek to where we are, how convenient */
        samples_to_do = -1;
      }
      /* do the actual seeking */
      if (samples_to_do >= 0)
      {
        debugMessage("render forward");
        while (samples_to_do > 0)
        {
          printf("%i\n", samples_to_do);
          l = min(576,samples_to_do);
          render_vgmstream(buffer,l,vgmstream);
          samples_to_do -= l;
          decode_pos_samples += l;
        }
        playback->output->flush(decode_seek);
        // reset eof flag
        eof = FALSE;
      }
      // reset decode_seek
      decode_seek = -1;
    }

    // ******************************************
    // Playback
    // ******************************************
    if (!eof)
    {
      // read data and pass onward
      samples_to_do = min(576,stream_length_samples - (decode_pos_samples + 576));
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
        if (vgmstream->loop_flag && fade_length_samples > 0 && !LOOPFOREVER) 
        {
            debugMessage("start fading");
            int samples_into_fade = decode_pos_samples - (stream_length_samples - fade_length_samples);
            if (samples_into_fade + samples_to_do > 0) {
                int j,k;
                for (j=0;j<samples_to_do;j++,samples_into_fade++) {
                    if (samples_into_fade > 0) {
                        double fadedness = (double)(fade_length_samples-samples_into_fade)/fade_length_samples;
                        for (k=0;k<vgmstream->channels;k++) {
                            buffer[j*vgmstream->channels+k] =
                                (short)(buffer[j*vgmstream->channels+k]*fadedness);
                        }
                    }
                }
            }
          }

          // pass it on
        // playback->pass_audio(playback,FMT_S16_LE,vgmstream->channels , l , buffer , playing );
        playback->output->write_audio(buffer, sizeof(buffer));
        decode_pos_samples += samples_to_do;
      }
    }
    else
    {
      debugMessage("waiting for track ending");
      // at EOF
      while (playback->output->buffer_playing())
        g_usleep(10000);

      debugMessage("track ending");
      playback->output->close_audio();
      playing = FALSE;
      // this effectively ends the loop
    }
  }
 exit_thread:
  decode_seek = -1;
  playing = FALSE;
  decode_pos_samples = 0;
  CLOSE_STREAM();

  return 0;
}

void vgmstream_play(InputPlayback *context, const char *filename, 
  VFSFile * file, int start_time, int stop_time, bool_t pause)
{
  debugMessage("start play");
  vgmstream = init_vgmstream_from_STREAMFILE(open_vfs(filename));

  if (!vgmstream || vgmstream->channels <= 0)  
  {
    printf("Channels are zero or couldn't init plugin\n");
    CLOSE_STREAM();
    goto end_thread;
  }

  if (!context->output->open_audio(FMT_S16_LE, vgmstream->sample_rate, vgmstream->channels))
  {
    printf("couldn't open audio device\n");
    CLOSE_STREAM();
    goto end_thread;
  }

  stream_length_samples = get_vgmstream_play_samples(LOOPCOUNT,FADESECONDS,FADEDELAYSECONDS,vgmstream);

  gint ms = (stream_length_samples * 1000LL) / vgmstream->sample_rate;
  gint rate   = vgmstream->sample_rate * 2 * vgmstream->channels;

  //set Tuple for track info
  Tuple * tuple = tuple_new_from_filename(filename);
  tuple_set_int(tuple, FIELD_LENGTH, NULL, ms);
  tuple_set_int(tuple, FIELD_BITRATE, NULL, rate);
  //tuple_set_str(tuple,FIELD_QUALITY , NULL, "lossless"); //eigen ?erweitern?
  context->set_tuple(context, tuple);

  context->set_pb_ready(context);
  vgmstream_play_loop(context);

end_thread:
  g_mutex_lock(ctrl_mutex);
  g_cond_signal(ctrl_cond);
  g_mutex_unlock(ctrl_mutex);
}

// void vgmstream_about()
// {
//   vgmstream_gui_about();
// }

void vgmstream_configure()
{
  vgmstream_gui_configure();
}

void vgmstream_init()
{
  debugMessage("init threads");
  // LoadSettings(&settings);
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

void vgmstream_stop(InputPlayback *context)
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
  context->output->close_audio();
  // cleanup
  CLOSE_STREAM();
}

void vgmstream_pause(InputPlayback *context,gshort paused)
{
  debugMessage("pause");
  context->output->pause(paused);
}

void vgmstream_mseek(InputPlayback *data, int ms)
{
  debugMessage("mseek");
  if (vgmstream)
  {
    decode_seek = ms;
    eof = FALSE;

    while (decode_seek != -1)
      g_usleep(10000);
  }
}

Tuple * vgmstream_probe_for_tuple(const gchar * filename, VFSFile * file)
{
  debugMessage("probe for tuple");
  Tuple * tuple = NULL;
  long length;

  if (!vgmstream)
    goto fail;

  tuple = tuple_new_from_filename(filename);

  length = get_vgmstream_play_samples(LOOPCOUNT,FADESECONDS,FADEDELAYSECONDS,vgmstream) * 1000LL / vgmstream->sample_rate;
  tuple_set_int(tuple, FIELD_LENGTH, NULL, length);
  return tuple;

fail:
    if (tuple) {
        tuple_unref(tuple);
    }
    return NULL;
}

// void vgmstream_file_info_box(const gchar *pFile) //optional
// {
//   char msg[1024] = {0};
//   VGMSTREAM *stream;

//   if ((stream = init_vgmstream_from_STREAMFILE(open_vfs(pFile))))
//   {
//     describe_vgmstream(stream,msg,sizeof(msg));

//     close_vgmstream(stream);

//     audacious_info_dialog("File information",msg,"OK",FALSE,NULL,NULL);
//   }
// }


/*
done:
  vgmstream_play


TODO:
  implement settings 
  free strings





Searches:
-search for format FMT_S16_LE  //function pass_audio
*/
