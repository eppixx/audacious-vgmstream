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

			//fading pasted from code
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
	vgmstream = init_vgmstream_from_STREAMFILE(open_vfs(context->filename))
	if (!vgmstream || vgmstream->channels <= 0)
	{
		CLOSE_STREAM()
		goto end_thread;
	}

	strcpy(strPlaying, context-filename) //copy file name

	//get samples
	stream_length_samples = get_vgmstream_play_samples(settings.loopcount, 
		settings.fadeseconds,
		settings.fadedelayseconds
		vgmstream)

	if (vgmstream->loop_flag)
	{
		//set the amount of samples that need fading
		fade_length_samples = settings.fadeseconds * vgmstream->sample_rate
	}
	else
	{
		//set default
		fade_length_samples = -1
	}

	//amount of time for track
	gint ms = (stream_length_samples * 1000LL) / vgmstream->sample_rate
	//the rate of the track
	gint rate = vgmstream->sample_rate * 2 * vgmstream->channels

	//create new tuple
	Tuple *tuple = tuple_new_from_filename(context->filename)
	//set tuple fields
	tuple_associate_int(tuple, FIELD_LENGTH, NULL, ms)
	tuple_associate_int(tuple, FIELD_BITRATE, NULL, rate)
	//set new tuple
	context->set_tuple(context, tuple)

	decode_thread = g_thread_self() //unnütz?
	context->set_pb_ready(context)  //set playback ready 
	vgmstream_play_loop(context)	//call own fkt

	end_thread:
	g_mutex_lock(ctrl_mutex)
	g_cond_signal(ctrl_cond)
	g_mutex_unlock(ctrl_mutex)
}

void vgmstream_stop(InputPlayback *context)
{
	if (vgmstream)
	{
		// schließe thread
		decode_seek = ende
		//warten bis geschlossen
		g_mutex_lock(ctrl_mutex)
		g_cond_wait(ctrl_cond, ctrl_mutex)
		g_mutex_unlock(ctrl_mutex)
	}
	context->output->close_audio()
	CLOSE_STREAM()
}

void vgmstream_pause(InputPlayback *context, gboolean pause)
{
	context->output->pause(pause)
}

void vgmstream_seek(InputPlayback *context, gint time)
{
	vgmstream_mseek(context, time * 1000)
}

Tuple * vgmstream_probe_for_tuple(const gchar * filename, VFSFile * file)
{
	VGMSTREAM *infostream = NULL
	Tuple *tuple = NULL
	long length

	infostream = init_vgmstream_from_STREAMFILE(open_vfs(filename))
	if (!infostream)
		goto fail

	tuple = tuple_new_from_filename(filename)

	gint samples = get_vgmstream_play_samples(settings.loopcount,
		settings.fadeseconds,
		settings.fadedelayseconds,
		infostream) 

	length = samples * 1000LL / infostream->sample_rate;)
	tuple_associate_int(tuple, FIELD_LENGTH, NULL, length)

	close vgmstream(infostream)

	return tuple

	fail:
	if (tuple)
		tuple_free(tuple)

	if (infostream)
		close_vgmstream(infostream)

	return NULL 
}

void vgmstream_file_info_box(const gchar *pFile) //optional
{
	char msg[1024] = {0} //puffer
	VGMSTREAM *stream

	if ((stream = init_vgmstream_from_STREAMFILE(open_vfs(pFile))))
	{
		describe_vgmstream(stream, msg, sizeof(msg)) //call von vgmstream
		close_vgmstream(stream) //schließe eben geöffneten stream wieder
		file_info_box("File information", msg, "OK", FALSE, NULL, NULL)
	}
}

/*
muss:
	char * const * extensions 		// dateiendungen vgm/unix/data.c	



*/

