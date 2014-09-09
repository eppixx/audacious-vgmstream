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


typedef struct _VFSSTREAMFILE
{
  STREAMFILE sf;
  VFSFile *vfsFile;
  off_t offset;
  char name[260];
  char realname[260];
} VFSSTREAMFILE;

static STREAMFILE *open_vfs_by_VFSFILE(VFSFile *file, const char *path);

static size_t read_vfs(VFSSTREAMFILE *streamfile, uint8_t *dest, off_t offset, size_t length)
{
  size_t sz;
  // if the offsets don't match, then we need to perform a seek
  if (streamfile->offset != offset) 
  {
    if (vfs_fseek(streamfile->vfsFile, offset, SEEK_SET) < 0)
      return 0;

    streamfile->offset = offset;
  }
  
  sz = vfs_fread(dest, 1, length, streamfile->vfsFile);
  // increment our current offset
  if (sz >= 0)
    streamfile->offset += sz;
  
  return sz;
}

static void close_vfs(VFSSTREAMFILE *streamfile)
{
  debugMessage("close_vfs");
  vfs_fclose(streamfile->vfsFile);
  free(streamfile);
}

static size_t get_size_vfs(VFSSTREAMFILE *streamfile)
{
  return vfs_fsize(streamfile->vfsFile);
}

static size_t get_offset_vfs(VFSSTREAMFILE *streamfile)
{
  return streamfile->offset;
}

static void get_name_vfs(VFSSTREAMFILE *streamfile, char *buffer, size_t length)
{
  strncpy(buffer, streamfile->name, length);
  buffer[length-1] = '\0';
}

static void get_realname_vfs(VFSSTREAMFILE *streamfile, char *buffer, size_t length)
{
    strncpy(buffer, streamfile->realname, length);
    buffer[length-1]='\0';
}

static STREAMFILE *open_vfs_impl(VFSSTREAMFILE *streamfile, const char * const filename, size_t buffersize) 
{
  if (!filename)
    return NULL;

  return open_vfs(filename);
}

static STREAMFILE *open_vfs_by_VFSFILE(VFSFile *file, const char *path)
{
  VFSSTREAMFILE *streamfile = malloc(sizeof(VFSSTREAMFILE));
  if (!streamfile)
    return NULL;
  
  // success, set our pointers
  memset(streamfile, 0, sizeof(VFSSTREAMFILE));

  streamfile->sf.read         = (void*)read_vfs;
  streamfile->sf.get_size     = (void*)get_size_vfs;
  streamfile->sf.get_offset   = (void*)get_offset_vfs;
  streamfile->sf.get_name     = (void*)get_name_vfs;
  streamfile->sf.get_realname = (void*)get_realname_vfs;
  streamfile->sf.open         = (void*)open_vfs_impl;
  streamfile->sf.close        = (void*)close_vfs;

  streamfile->vfsFile = file;
  streamfile->offset  = 0;
  strncpy(streamfile->name, path, sizeof(streamfile->name));
  streamfile->name[sizeof(streamfile->name)-1] = '\0';
  {
      gchar* realname = g_filename_from_uri(path, NULL, NULL);
      strncpy(streamfile->realname, realname, sizeof(streamfile->realname));
      streamfile->realname[sizeof(streamfile->realname)-1] = '\0';
      g_free(realname);
  }
  
  return &streamfile->sf;
}

STREAMFILE *open_vfs(const char *path)
{
  VFSFile *vfsFile = vfs_fopen(path, "rb");
  if (!vfsFile)
    return NULL;

  return open_vfs_by_VFSFILE(vfsFile, path);
}
