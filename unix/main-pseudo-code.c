void vgmstream_play_loop()
{
	init

	//seeking
	while (play track)
	{
		if (decode_seek == ende)
		{
			goto ende_thread
		}
		else if (decode_seek >= 0)
		{
			needed_sample = rechne ms zu samples //vgm wird in samples gegliedert
			if (needed_sample vor decoded_sample)
			{
				reopen file
			}
			else if (decoded_sample vor needed_sample)
			{
				samples_to_do = errechne samples die vor mir liegen
			}
			sonst
			{
				samples_to_do = -1 //weil wir da sind wo wir wollen
			}
		}

		if (samples_to_do > 0)
		{
			//annähern an needed_sample bzw abarbeiten von samples_to_do
			while (samples_to_do > 0)
			{
				render_vgmstream(puffer, samples_to_do, vgmstream)
				samples_to_do--
				decoded_sample++
			}
			output_flush //deprecated only sets decode_seek
			reset eof
		}
		reset decode_seek
	}

	//playback
	if (! eof)
	{
		//lese daten und leite weiter
		sampels_to_do = sampels_gesamt - (decoded_sample+576)
		l = samples_to_do * stream_channels * 2

		if (samples_to_do == 0)
		{
			eof = 1;
		}
		sonst
		{
			render_vgmstream(puffer, samples_to_do, vgmstream)

			//fading
			if (vgmstream->loop_flag && fade_length_samples > 0 && !loop_forever) 
			{
	            int samples_into_fade = decode_pos_samples - (stream_length_samples - fade_length_samples);
	            if (samples_into_fade + samples_to_do > 0) 
	            {
	                int j,k;
	                for (j=0;j<samples_to_do;j++,samples_into_fade++) 
	                {
	                    if (samples_into_fade > 0) 
	                    {
	                        double fadedness = (double)(fade_length_samples-samples_into_fade)/fade_length_samples;
	                        for (k=0;k<vgmstream->channels;k++) 
	                        {
	                            buffer[j*vgmstream->channels+k] = (short)(buffer[j*vgmstream->channels+k]*fadedness);
	                        }
	                    }
	                }
	            }
	        }

	        write_audio()
	        decoded_sample += samples_to_do
		}
	}
	sonst
	{
		//eof erreicht
		while (buffer playing) //buffer playing nicht mehr vorhanden
		{
			g_usleep(10000) //thread sleep for 10000 ms
		}
		playing = false 
	}
	ende_thread:
	set vars 
	CLOSE_STREAM()
}

void vgmstream_init()
{
	//lade einstellungen
	//erstelle neuen thread
	//erstelle neuen mutex

	LoadSettiings(&settings)
	ctrl_cond = g_cond_new()
	//http://www.gtk.org/api/2.6/glib/glib-Threads.html#g-cond-new
	ctrl_mutex = g_mutex_new()
	//http://www.gtk.org/api/2.6/glib/glib-Threads.html#g-mutex-new
}

void vgmstream_destroy()
{
	//schließe thread
	//schließe mutex 

	g_cond_free(ctrl_cond);
	ctrl_cond = NULL;
	g_mutex_free(ctrl_mutex);
	ctrl_mutex = NULL;
}

void vgmstream_mseek(InputPlayback *data, gulong ms)
{
	if (vgmstream)
	{
		decode_seek = ms
		eof = 0

		while (decode_seek != -1)
			g_usleep(10000)
	}
}

void vgmstream_play(InputPlayback *context)
{

}

void vgmstream_pause(InputPlayback *context)//, gshort paused)
{
	context->output->pause(true)
}

void vgmstream_seek(InputPlayback *context, gint time)
{
	vgmstream_mseek(context, time * 1000)
}

Tuple * vgmstream_probe_for_tuple(const gchar * filename, VFSFile * file)
{

}

void vgmstream_file_info_box(const gchar *pFile)
{
	VGMSTREAM *stream



}

/*
muss:
	char * const * extensions 		// dateiendungen vgm/unix/data.c	



*/

