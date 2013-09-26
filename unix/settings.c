#include <gtk/gtk.h>

#include <audacious/plugin.h>
#include <audacious/misc.h>

#include "settings.h"
#include "version.h"

#define CFG_ID "vgmstream"  //ID for storing in audacious

// //default-values for Settings
#define DEFAULT_FADE_LENGTH "0"   //tenth seconds so we can store in int
#define DEFAULT_FADE_DELAY "30"   //tenth seconds so we can store in int
#define DEFAULT_LOOP_COUNT "2"
#define DEFUALT_LOOP_FOREVER "0"

static const gchar* const defaults[] = 
{
  "loop_forever", DEFUALT_LOOP_FOREVER,
  "loop_count",   DEFAULT_LOOP_COUNT,
  "fade_length",  DEFAULT_FADE_LENGTH,
  "fade_delay",   DEFAULT_FADE_DELAY,
  NULL 
};

const char vgmstream_about[] =
{
  "audacious-vgmstream version: " AUDACIOUSVGMSTREAM_VERSION "\n\n"
  "audacious-vgmstream originally written by Todd Jeffreys (http://voidpointer.org/)\n"
  "ported to audacious 3 by Thomas Eppers\n"
  "vgmstream written by hcs, FastElbja, manakoAT, and bxaimc (http://www.sf.net/projects/vgmstream)"
};

Settings vgmstream_cfg;

static GtkWidget *window;        // Configure Window
static GtkWidget *loop_count;    // Spinbutton
static GtkWidget *fade_length;   // SpinButton
static GtkWidget *fade_delay;    // SpinButton
static GtkWidget *loop_forever;  // CheckBox

static void on_loop_forever_changed();
static void on_cancel();
static void on_OK();

//uses the store-mechanism from audacious
//because audacious can't save float we store them in integer and
//multiply with 0.1 to get the desired float
void vgmstream_cfg_load()
{
  debugMessage("cfg_load called");
  aud_config_set_defaults(CFG_ID, defaults);

  vgmstream_cfg.loop_forever = aud_get_bool(CFG_ID, "loop_forever");
  vgmstream_cfg.loop_count   = aud_get_int (CFG_ID, "loop_count");
  vgmstream_cfg.fade_length  = aud_get_int (CFG_ID, "fade_length") * 0.1f;
  vgmstream_cfg.fade_delay   = aud_get_int (CFG_ID, "fade_delay")  * 0.1f;
}

//we multiply with 10 to be able to store integer
void vgmstream_cfg_safe()
{
  debugMessage("cfg_save called");
  aud_set_bool(CFG_ID, "loop_forever", vgmstream_cfg.loop_forever);
  aud_set_int(CFG_ID, "loop_count",    vgmstream_cfg.loop_count);
  aud_set_int(CFG_ID, "fade_length",   (gint)vgmstream_cfg.fade_length * 10);
  aud_set_int(CFG_ID, "fade_delay",    (gint)vgmstream_cfg.fade_delay  * 10);
}

void vgmstream_cfg_ui()
{
  debugMessage("called configure");
  GtkWidget     *hbox;        // horizontal Box
  GtkWidget     *vbox;        // vertical Box

  GtkWidget     *bbox;        // Box for buttons
  GtkWidget     *ok;          // OK-Button
  GtkWidget     *cancel;      // Cancel-Button
  
  GtkWidget     *label;       // label for strings 
  GtkAdjustment *adjustment;  // adjustment for SpinButton
  
  if (window)
  {
    gtk_window_present(GTK_WINDOW(window));
    return;
  }

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG );
  gtk_window_set_title(GTK_WINDOW(window), (gchar *)"VGMStream Decoder - Config");
  gtk_container_set_border_width(GTK_CONTAINER(window), 15);

  vbox = gtk_vbox_new (FALSE, 5);

  // loop forever
  loop_forever = gtk_check_button_new_with_label("Loop Forever");
  g_signal_connect (loop_forever, "toggled", G_CALLBACK (on_loop_forever_changed), NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(loop_forever), vgmstream_cfg.loop_forever);
  gtk_container_add(GTK_CONTAINER (vbox), loop_forever);

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
  adjustment = gtk_adjustment_new(vgmstream_cfg.fade_length, 0, 10, 0.2, 0, 0);
  fade_length = gtk_spin_button_new(adjustment, 0.2, 1);
  hbox = gtk_hbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER (hbox), label);
  gtk_container_add(GTK_CONTAINER (hbox), fade_length);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  // fade delay
  label = gtk_label_new("Fade delay");
  adjustment = gtk_adjustment_new(vgmstream_cfg.fade_delay, 0, 10, 0.2, 0, 0);
  fade_delay = gtk_spin_button_new(adjustment, 0.2, 1);
  hbox = gtk_hbox_new(FALSE,5);
  gtk_container_add(GTK_CONTAINER (hbox), label);
  gtk_container_add(GTK_CONTAINER (hbox), fade_delay);
  gtk_container_add(GTK_CONTAINER (vbox), hbox);

  //buttons
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

  if (vgmstream_cfg.loop_forever)
  {
    gtk_widget_set_sensitive(loop_count, FALSE);
    gtk_widget_set_sensitive(fade_length, FALSE);
    gtk_widget_set_sensitive(fade_delay, FALSE);
  }

  gtk_widget_show_all (window);
}

//when the loop_forever checkbox is activated the other options
//are disabled/unsensitive
static void on_loop_forever_changed()
{
  debugMessage("on loop forever changed");
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(loop_forever)))
  {
    debugMessage("unsensitive");
    gtk_widget_set_sensitive(loop_count, FALSE);
    gtk_widget_set_sensitive(fade_length, FALSE);
    gtk_widget_set_sensitive(fade_delay, FALSE);
  }
  else
  {
    debugMessage("sensitive");
    gtk_widget_set_sensitive(loop_count, TRUE);
    gtk_widget_set_sensitive(fade_length, TRUE);
    gtk_widget_set_sensitive(fade_delay, TRUE);
  }
}

static void on_cancel()
{
  debugMessage("clicked Cancel on configure");
  window = NULL;
}

static void on_OK()
{
  debugMessage("clicked OK on configure");
  vgmstream_cfg.loop_forever = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(loop_forever));
  vgmstream_cfg.loop_count   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(loop_count));
  vgmstream_cfg.fade_length  = gtk_spin_button_get_value(GTK_SPIN_BUTTON(fade_length));
  vgmstream_cfg.fade_delay   = gtk_spin_button_get_value(GTK_SPIN_BUTTON(fade_delay));
  vgmstream_cfg_safe();
  window = NULL;
}

void vgmstream_cfg_about()
{
  debugMessage("called cfg_about");

  static GtkWidget *about_window;
  audgui_simple_message (&about_window, GTK_MESSAGE_INFO,
    "About the VGMStream Decoder", vgmstream_about);
}

void debugMessage(const char *str)
{
  #ifdef DEBUG
  printf("Debug::%s\n", str);
  #endif
}
