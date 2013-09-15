#include <gtk/gtk.h>

#include <audacious/plugin.h>
#include <audacious/misc.h>

#include "settings.h"
#include "version.h"

#define CFG_ID "vgmstream"

Settings vgmstream_cfg;

static GtkWidget *window;        // Configure Window
static GtkWidget *loop_count;    // Spinbutton
static GtkWidget *fade_length;   // SpinButton
static GtkWidget *fade_delay;    // SpinButton
static GtkWidget *loop_forever;  // CheckBox

static GtkWidget *window_about;  // About Window

const char vgmstream_about[] =
{
  "audacious-vgmstream version: " AUDACIOUSVGMSTREAM_VERSION "\n\n"
  "audacious-vgmstream originally written by Todd Jeffreys (http://voidpointer.org/)\nported to aduacious 3 by Thomas Eppers\n"
  "vgmstream written by hcs, FastElbja, manakoAT, and bxaimc (http://www.sf.net/projects/vgmstream)"
};

static const gchar* const defaults[] = 
{
  "fade_seconds", "0",
  "fade_delayseconds", "0",
  "loop_count", "2",
  "loop_forever", "FALSE",
  NULL 
};

static void on_OK();
static void on_cancel();

void vgmstream_cfg_load()
{
  debugMessage("cfg_load called");
  aud_config_set_defaults(CFG_ID, defaults);

  vgmstream_cfg.fade_length = aud_get_int(CFG_ID, "fade_length");
  vgmstream_cfg.fade_delay = aud_get_int(CFG_ID, "fade_delay");
  vgmstream_cfg.loop_count = aud_get_int(CFG_ID, "loop_count");
  vgmstream_cfg.loop_forever = aud_get_bool(CFG_ID, "loop_forever");
}

void vgmstream_cfg_safe()
{
  debugMessage("cfg_save called");
  aud_set_int(CFG_ID, "fade_length", vgmstream_cfg.fade_length);
  aud_set_int(CFG_ID, "fade_delay", vgmstream_cfg.fade_delay);
  aud_set_int(CFG_ID, "loop_count", vgmstream_cfg.loop_count);
  aud_set_bool(CFG_ID, "loop_forever", vgmstream_cfg.loop_forever);
}

void vgmstream_cfg_ui()
{
  debugMessage("called configure");
  GtkWidget     *hbox;        // horizontal Box
  GtkWidget     *vbox;        // vertical Box

  GtkWidget     *bbox;        // Box for buttons
  GtkWidget     *ok;          // OK-Button
  GtkWidget     *cancel;      // Cancel-Button
  
  GtkWidget     *label;
  GtkAdjustment *adjustment;  // adjustment for SpinButton
  GtkWidget     *button;      // SpinButton
  
  if (window)
  {
    gtk_window_present(window);
    return;
  }

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG );
  gtk_window_set_title(GTK_WINDOW(window), (gchar *)"VGMStream Decoder - Config");
  gtk_container_set_border_width(GTK_CONTAINER(window), 25);

  vbox = gtk_vbox_new (FALSE, 5);

  // loop count
  label = gtk_label_new("Loop count");
  adjustment = gtk_adjustment_new(vgmstream_cfg.loop_count, 1, 10, 1, 0, 0);
  loop_count = gtk_spin_button_new(adjustment, 1.0, 0);
  hbox = gtk_hbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER (hbox), label);
  gtk_container_add(GTK_CONTAINER (hbox), loop_count);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  // fade length
  label = gtk_label_new("Fade length");
  adjustment = gtk_adjustment_new(vgmstream_cfg.fade_length/10, 0, 10, 0.2, 0, 0);
  fade_length = gtk_spin_button_new(adjustment, 0.2, 1);
  hbox = gtk_hbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER (hbox), label);
  gtk_container_add(GTK_CONTAINER (hbox), fade_length);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  // fade delay
  label = gtk_label_new("Fade delay");
  adjustment = gtk_adjustment_new(vgmstream_cfg.fade_delay/10, 0, 10, 0.2, 0, 0);
  fade_delay = gtk_spin_button_new(adjustment, 0.2, 1);
  hbox = gtk_hbox_new(FALSE,5);
  gtk_container_add(GTK_CONTAINER (hbox), label);
  gtk_container_add(GTK_CONTAINER (hbox), fade_delay);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  // loop forever
  loop_forever = gtk_check_button_new_with_label("Loop Forever");
  gtk_toggle_button_set_active(loop_forever, vgmstream_cfg.loop_forever);
  gtk_container_add(GTK_CONTAINER (vbox), loop_forever);

  bbox = gtk_hbox_new(TRUE, 5);
  // cancel button
  cancel = gtk_button_new_with_label((gchar *)"Cancel");
  g_signal_connect (cancel, "clicked", G_CALLBACK (on_cancel), NULL);
  g_signal_connect_swapped (cancel, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_container_add(GTK_CONTAINER (bbox), cancel);
  
  // ok button
  ok = gtk_button_new_with_label((gchar *)"OK");
  g_signal_connect (ok, "clicked", G_CALLBACK (on_OK), NULL);
  g_signal_connect_swapped (ok, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_container_add(GTK_CONTAINER (bbox), ok);
  gtk_container_add(GTK_CONTAINER (vbox), bbox);

  gtk_container_add(GTK_CONTAINER (window), vbox);
  gtk_widget_show_all (window);
}

static void on_OK()
{
  debugMessage("clicked OK on configure");
  vgmstream_cfg.loop_count = gtk_spin_button_get_value_as_int(loop_count);
  vgmstream_cfg.fade_delay = gtk_spin_button_get_value(fade_delay)*10;
  vgmstream_cfg.fade_length = gtk_spin_button_get_value(fade_length)*10;
  vgmstream_cfg.loop_forever = gtk_toggle_button_get_active(loop_forever);
  vgmstream_cfg_safe();
  window = NULL;
}

static void on_cancel()
{
  debugMessage("clicked Cancel on configure");
  window = NULL;
}

void vgmstream_cfg_about()
{
  debugMessage("called cfg_about");

  static GtkWidget * about_box;
  audgui_simple_message (& about_box, GTK_MESSAGE_INFO,
    "About the VGMStream Decoder", vgmstream_about);
}

void debugMessage(char *str)
{
  if (DEBUG)
  {
    printf("%s\n", str);
  }
}
