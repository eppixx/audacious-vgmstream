#ifndef __SETTINGS__
#define __SETTINGS__

#include <glib.h>

// function for debug output
#define DEBUG TRUE
void debugMessage(char *str);

// temp till real settings
#define LOOPCOUNT 2
#define FADESECONDS 0
#define FADEDELAYSECONDS 0
#define LOOPFOREVER 0

//default-values
#define DEFAULT_FADE_SECONDS 0
#define DEFAULT_FADE_DELAY_SECONDS 0
#define DEFAULT_LOOP_COUNT 2
#define DEFUALT_LOOP_FOREVER FALSE

typedef struct
{
	gfloat fade_length;
	gfloat fade_delay;
	gint loop_count;
	gboolean loop_forever;
} Settings;

extern Settings vgmstream_cfg;

extern const char vgmstream_about[];

void vgmstream_cfg_load();
void vgmstream_cfg_save();
void vgmstream_cfg_ui();
void vgmstream_cfg_about();

#endif
