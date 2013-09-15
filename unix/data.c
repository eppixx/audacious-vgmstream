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


void vgmstream_init();
void vgmstream_gui_about();
void vgmstream_cfg_ui();
void vgmstream_destroy();

Tuple * vgmstream_probe_for_tuple(const gchar *uri, VFSFile *fd);
void vgmstream_file_info_box(const gchar *pFile);
void vgmstream_play(InputPlayback *context);
void vgmstream_pause(InputPlayback *context,gshort paused);
void vgmstream_mseek(InputPlayback *context, int ms);
void vgmstream_stop(InputPlayback *context);

static const gchar *vgmstream_exts [] = 
{
  "2dx9",
  "2pfs",

  "aax",
  "acm",
  "adm",
  "adp",
  "adpcm",
  "ads",
  "adx",
  "afc",
  "agsc",
  "ahx",
  "aifc",
  "aix",
  "amts",
  "as4",
  "asd",
  "asf",
  "asr",
  "ass",
  "ast",
  "aud",
  "aus",

  "baka",
  "baf",
  "bar",
  "bcwav",
  "bg00",
  "bgw",
  "bh2pcm",
  "bmdx",
  "bms",
  "bns",
  "bnsf",
  "bo2",
  "brstm",
  "brstmspm",
  "bvg",

  "caf",
  "capdsp",
  "cbd2",
  "ccc",
  "cfn",
  "ckd",
  "cnk",
  "cps",

  "dcs",
  "de2",
  "ddsp",
  "dmsg",
  "dsp",
  "dspw",
  "dtk",
  "dvi",
  "dxh",

  "eam",
  "emff",
  "enth",

  "fag",
  "filp",
  "fsb",

  "gbts",
  "gca",
  "gcm",
  "gcub",
  "gcw",
  "genh",
  "gms",
  "gsb",

  "hgc1",
  "his",
  "hlwav",
  "hps",
  "hsf",
  "hwas",

  "iab",
  "idsp",
  "idvi",
  "ikm",
  "ild",
  "int",
  "isd",
  "ivaud",
  "ivag",
  "ivb",

  "joe",
  "jstm",

  "kces",
  "kcey",
  "khv",
  "klbs",
  "kovs",
  "kraw",

  "leg",
  "logg",
  "lpcm",
  "lps",
  "lsf",
  "lwav",

  "matx",
  "mcg",
  "mi4",
  "mib",
  "mic",
  "mihb",
  "mpdsp",
  "mpds",
  "msa",
  "msf",
  "mss",
  "msvp",
  "mtaf",
  "mus",
  "musc",
  "musx",
  "mwv",
  "mxst",
  "myspd",

  "ndp",
  "ngca",
  "npsf",
  "nwa",

  "omu",

  "p2bt",
  "p3d",
  "past",
  "pcm",
  "pdt",
  "pnb",
  "pos",
  "ps2stm",
  "psh",
  "psnd",
  "psw",

  "ras",
  "raw",
  "rkv",
  "rnd",
  "rrds",
  "rsd",
  "rsf",
  "rstm",
  "rvws",
  "rwar",
  "rwav",
  "rws",
  "rwsd",
  "rwx",
  "rxw",

  "s14",
  "sab",
  "sad",
  "sap",
  "sc",
  "scd",
  "sck",
  "sd9",
  "sdt",
  "seg",
  "sf0",
  "sfl",
  "sfs",
  "sfx",
  "sgb",
  "sgd",
  "sl3",
  "sli",
  "smp",
  "smpl",
  "snd",
  "snds",
  "sng",
  "sns",
  "spd",
  "spm",
  "sps",
  "spsd",
  "spw",
  "ss2",
  "ss3",
  "ss7",
  "ssm",
  "sss",
  "ster",
  "stma",
  "str",
  "strm",
  "sts",
  "stx",
  "svag",
  "svs",
  "swav",
  "swd",

  "tec",
  "thp",
  "tk1",
  "tk5",
  "tra",
  "tun",
  "tydsp",

  "um3",

  "vag",
  "vas",
  "vawx",
  "vb",
  "vgs",
  "vgv",
  "vig",
  "vms",
  "voi",
  "vpk",
  "vs",
  "vsf",

  "waa",
  "wac",
  "wad",
  "wam",
  "wavm",
  "wb",
  "wii",
  "wmus",
  "wp2",
  "wpd",
  "wsd",
  "wsi",
  "wvs",

  "xa",
  "xa2",
  "xa30",
  "xau",
  "xmu",
  "xnb",
  "xsf",
  "xss",
  "xvag",
  "xvas",
  "xwav",
  "xwb",

  "ydsp",
  "ymf",

  "zsd",
  "zwdsp",
  /* terminator */
  NULL
};


AUD_INPUT_PLUGIN
(
  //Common Fields
  .name = "VGMStream Decoder",
  .init = vgmstream_init,
  // .about_text = vgmstream_about,    //works in version 3.3 and later
  .about = vgmstream_cfg_about,
  .configure = vgmstream_cfg_ui,
  .cleanup = vgmstream_destroy,
  
  //InputPlugin Fields
  .extensions = vgmstream_exts,
  .probe_for_tuple = vgmstream_probe_for_tuple,
  .play = vgmstream_play,
  .pause = vgmstream_pause,
  .mseek = vgmstream_mseek,
  .stop = vgmstream_stop,
)


