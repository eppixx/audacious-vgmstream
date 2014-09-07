#ifndef __SETTINGS__
#define __SETTINGS__

#include <glib.h>

// function for debug output
// #define DEBUG 1
void debugMessage(const char *str);

//defines struct for Settings
typedef struct
{
	gboolean loop_forever;
	gint     loop_count;
	gfloat   fade_length;
	gfloat   fade_delay;
} Settings;

extern Settings vgmstream_cfg;			//struct that stores settings
extern const char vgmstream_about[];	//string for about dialog

void vgmstream_cfg_load();
void vgmstream_cfg_save();
void vgmstream_cfg_ui();

#endif
