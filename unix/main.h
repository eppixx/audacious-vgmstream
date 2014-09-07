gboolean vgmstream_init();
void vgmstream_gui_about();
void vgmstream_cfg_ui();

Tuple *vgmstream_probe_for_tuple(const gchar *uri, VFSFile *fd);
gboolean vgmstream_play(const gchar *filename, VFSFile *file);
