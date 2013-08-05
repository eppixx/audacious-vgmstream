#ifndef __SETTINGS__
#define __SETTINGS__

// function for debug output
#define DEBUG 1
void debugMessage(char *str);

// temp till real settings
#define LOOPCOUNT 1
#define FADESECONDS 0
#define FADEDELAYSECONDS 0

typedef struct _SETTINGS
{
  int loopcount;
  int fadeseconds;
  int fadedelayseconds;
} SETTINGS,*PSETTINGS,*LPSETTINGS;

void DefaultSettings(LPSETTINGS pSettings);
int LoadSettings(LPSETTINGS pSettings);
int SaveSettings(LPSETTINGS pSettings);



#endif
