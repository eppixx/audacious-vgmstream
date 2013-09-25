#ifndef __SETTINGS__
#define __SETTINGS__

#include <glib.h>

// function for debug output
#define DEBUG TRUE
void debugMessage(char *str);

//default-values
#define DEFAULT_FADE_SECONDS 0
#define DEFAULT_FADE_DELAY_SECONDS 3
#define DEFAULT_LOOP_COUNT 2
#define DEFUALT_LOOP_FOREVER FALSE

typedef struct
{
	gboolean loop_forever;
	gint loop_count;
	gfloat fade_length;
	gfloat fade_delay;
} Settings;

extern Settings vgmstream_cfg;
extern const char vgmstream_about[];

void vgmstream_cfg_load();
void vgmstream_cfg_save();
void vgmstream_cfg_ui();
void vgmstream_cfg_about();

#endif
