#include "settings.h"
#include <audacious/plugin.h>

#define CUBE_CONFIG_TAG "vgmstream"

// typedef struct 
// {
//   int loopcount;
//   int fadeseconds;
//   int fadedelayseconds;
// } Settings;

// // static ConfigDb *GetConfigFile( void )
// // {
//   // ConfigDb *cfg = aud_cfg_db_open();
//   // return cfg;
// static Settings *GetConfigFile( void )
// {
//   struct Settings *s;
//   memset(s , 0, sizeof(Settings));
//   s->loopcount = 1;
//   s->fadeseconds = 0;
//   s->fadedelayseconds = 0;
//   return s;
// }


// void DefaultSettings(LPSETTINGS pSettings)
// {
//   memset(pSettings,0,sizeof(SETTINGS));
  
//   pSettings->loopcount = 1;
//   pSettings->fadeseconds = 0;
//   pSettings->fadedelayseconds = 0;
// }

// #define LOOPCOUNT_NAME "loopcount"
// #define FADESECOND_NAME "fadeseconds"
// #define FADEDELAYSECOND_NAME "fadedelayseconds"

// int LoadSettings(LPSETTINGS pSettings)
// {
//   ConfigDb *cfg = GetConfigFile();
//   if (!cfg)
//   {
//     DefaultSettings(pSettings);
//   }
//   else
//   {  
//     int bRet = (aud_cfg_db_get_int(cfg,CUBE_CONFIG_TAG,LOOPCOUNT_NAME,&pSettings->loopcount) && 
// 		aud_cfg_db_get_int(cfg,CUBE_CONFIG_TAG,FADESECOND_NAME,&pSettings->fadeseconds) &&
// 		aud_cfg_db_get_int(cfg,CUBE_CONFIG_TAG,FADEDELAYSECOND_NAME,&pSettings->fadedelayseconds));
    
//     aud_cfg_db_close(cfg); 
    
//     // check if reading one value failed.  If so, then use defaults
//     if (!bRet)
//       DefaultSettings(pSettings);
//   }
//   return 1;
// }

// int SaveSettings(LPSETTINGS pSettings)
// {
//   ConfigDb *cfg = GetConfigFile();
//   if (!cfg)
//     return 0;

//   aud_cfg_db_set_int(cfg,CUBE_CONFIG_TAG,LOOPCOUNT_NAME,pSettings->loopcount);
//   aud_cfg_db_set_int(cfg,CUBE_CONFIG_TAG,FADESECOND_NAME,pSettings->fadeseconds);
//   aud_cfg_db_set_int(cfg,CUBE_CONFIG_TAG,FADEDELAYSECOND_NAME,pSettings->fadedelayseconds);

//   aud_cfg_db_close(cfg);
//   return 1;
// }
