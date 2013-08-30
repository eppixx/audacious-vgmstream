// #include <audacious/util.h>
// #include <audacious/configdb.h>
// #include <glib.h>
#include <gtk/gtk.h>
// #include "gui.h"
// #include "version.h"
// #include "settings.h"
// #include <stdio.h>
// #include <stdarg.h>

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

// extern SETTINGS settings;
static GtkWidget *about_box;

// static void DisplayError(char *pMsg,...)
// {
//   GtkWidget *mbox_win,
//     *mbox_vbox1,
//     *mbox_vbox2,
//     *mbox_frame,
//     *mbox_label,
//     *mbox_bbox,
//     *mbox_ok;
//   va_list vlist;
//   char message[1024];

//   va_start(vlist,pMsg);
//   vsnprintf(message,sizeof(message),pMsg,vlist);
//   va_end(vlist);

//   mbox_win = gtk_window_new( GTK_WINDOW_TOPLEVEL );
//   gtk_window_set_type_hint( GTK_WINDOW(mbox_win), GDK_WINDOW_TYPE_HINT_DIALOG );
//   gtk_signal_connect(GTK_OBJECT(mbox_win), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &mbox_win);
//   gtk_window_set_title(GTK_WINDOW(mbox_win), (gchar *)"vgmstream file information");
//   gtk_window_set_policy(GTK_WINDOW(mbox_win), FALSE, FALSE, FALSE);
//   gtk_container_border_width(GTK_CONTAINER(mbox_win), 10);

//   mbox_vbox1 = gtk_vbox_new(FALSE, 10);
//   gtk_container_add(GTK_CONTAINER(mbox_win), mbox_vbox1);

//   mbox_frame = gtk_frame_new((gchar *)" vgmstream error ");
//   gtk_container_set_border_width(GTK_CONTAINER(mbox_frame), 5);
//   gtk_box_pack_start(GTK_BOX(mbox_vbox1), mbox_frame, FALSE, FALSE, 0);

//   mbox_vbox2 = gtk_vbox_new(FALSE, 10);
//   gtk_container_set_border_width(GTK_CONTAINER(mbox_vbox2), 5);
//   gtk_container_add(GTK_CONTAINER(mbox_frame), mbox_vbox2);

//   mbox_label = gtk_label_new((gchar *)message);
//   gtk_misc_set_alignment(GTK_MISC(mbox_label), 0, 0);
//   gtk_label_set_justify(GTK_LABEL(mbox_label), GTK_JUSTIFY_LEFT);
//   gtk_box_pack_start(GTK_BOX(mbox_vbox2), mbox_label, TRUE, TRUE, 0);
//   gtk_widget_show(mbox_label);

//   mbox_bbox = gtk_hbutton_box_new();
//   gtk_button_box_set_layout(GTK_BUTTON_BOX(mbox_bbox), GTK_BUTTONBOX_SPREAD);
//   gtk_button_box_set_spacing(GTK_BUTTON_BOX(mbox_bbox), 5);
//   gtk_box_pack_start(GTK_BOX(mbox_vbox2), mbox_bbox, FALSE, FALSE, 0);

//   mbox_ok = gtk_button_new_with_label((gchar *)"OK");
//   gtk_signal_connect_object(GTK_OBJECT(mbox_ok), "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(mbox_win));
//   GTK_WIDGET_SET_FLAGS(mbox_ok, GTK_CAN_DEFAULT);
//   gtk_box_pack_start(GTK_BOX(mbox_bbox), mbox_ok, TRUE, TRUE, 0);
//   gtk_widget_show(mbox_ok);
//   gtk_widget_grab_default(mbox_ok);
  
//   gtk_widget_show(mbox_bbox);
//   gtk_widget_show(mbox_vbox2);
//   gtk_widget_show(mbox_frame);
//   gtk_widget_show(mbox_vbox1);
//   gtk_widget_show(mbox_win);
// }

// static int ToInt(const char *pText,int *val)
// {
//   char *end;
//   if (!pText) return 0;

//   *val = strtol(pText,&end,10);
//   if (!end || *end)
//     return 0;
  
//   return 1;
// }

// static void OnOK()
// {
//   // SETTINGS s;
//   // update my variables
//   if (!ToInt(gtk_entry_get_text(GTK_ENTRY(loopcount_win)),LOOPCOUNT))
//   {
//     DisplayError("Invalid loop times entry.");
//     return;
//   }
//   if (!ToInt(gtk_entry_get_text(GTK_ENTRY(fadeseconds_win)),FADESECONDS))
//   {
//     DisplayError("Invalid fade delay entry.");
//     return;
//   }
//   if (!ToInt(gtk_entry_get_text(GTK_ENTRY(fadedelayseconds_win)),FADEDELAYSECONDS))
//   {
//     DisplayError("Invalid fade length entry.");
//     return;
//   }

//   // if (SaveSettings(&s))
//   // {
//   //   // settings = s;		/* update local copy */
//   //   gtk_widget_destroy(config_win);
//   // }
//   // else
//   // {
//   //   DisplayError("Unable to save settings\n");
//   // }
// }

// void vgmstream_gui_about()
// {
//   debugMessage("called gui_about");
//   if (about_box)
//   {
//     gdk_window_raise(about_box->window);
//     return;
//   }
  
//   about_box = audacious_info_dialog(
//     (gchar *) "About VGMStream Decoder",
//     (gchar *) "[ VGMStream Decoder ]\n\n"
//     "audacious-vgmstream version: " AUDACIOUSVGMSTREAM_VERSION "\n\n"
//     "audacious-vgmstream written by Todd Jeffreys (http://voidpointer.org/)\n"
//     "vgmstream written by hcs, FastElbja, manakoAT, and bxaimc (http://www.sf.net/projects/vgmstream)",
//     (gchar *) "OK",
//     FALSE, NULL, NULL);
//   gtk_signal_connect(GTK_OBJECT(about_box), "destroy",
//                      GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_box);
  
// }
