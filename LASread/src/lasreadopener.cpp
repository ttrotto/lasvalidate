/*
===============================================================================

  FILE:  lasreadopener.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2005-2013, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/
#include "lasreadopener.hpp"
#include "lasreader.hpp"
#include "laswaveformreader.hpp"

#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#define DIRECTORY_SLASH '\\'
#else
#define DIRECTORY_SLASH '/'
#endif

//BOOL LASreadOpener::is_piped() const
//{
//  return (!file_names && use_stdin);
//}

BOOL LASreadOpener::is_active() const
{
  return ((file_name_current < file_name_number));
}

void LASreadOpener::reset()
{
  file_name_current = 0;
  file_name = 0;
}

LASreader* LASreadOpener::open(U32 file_name_current)
{
  if (file_names)
  {
    if (file_name_current == file_name_number)
    {
      return 0;
    }
    else
    {
      file_name = file_names[file_name_current];
    }
#ifdef LASZIP_ENABLED
    if (strstr(file_name, ".las") || strstr(file_name, ".laz") || strstr(file_name, ".LAS") || strstr(file_name, ".LAZ"))
#else
    if (strstr(file_name, ".las") || strstr(file_name, ".LAS"))
#endif
    {
      LASreader* lasreader = new LASreader();
      if (!lasreader->open(file_name, io_ibuffer_size))
      {
        if (!lasreader->header.fails)
        {
          fprintf(stderr,"ERROR: cannot open lasreader with file name '%s'\n", file_name);
          delete lasreader;
          return 0;
        }
      }
      return lasreader;
    }
#ifdef LASZIP_ENABLED
    else
#else
    else if (strstr(file_name, ".laz") || strstr(file_name, ".LAZ"))
    {
      fprintf(stderr,"ERROR: LASread not compiled with LASZIP_ENABLE preprocessor and therefore\n");
      fprintf(stderr,"       does not support LAZ files like '%s'\n", file_name);
      return 0;
    }
    else
#endif
    {
      fprintf(stderr,"ERROR: unknown file extension '%s'\n", file_name);
      return 0;
    }
  }
  return 0;
}

LASwaveformreader* LASreadOpener::open_waveform(const LASheader* lasheader)
{
  if ((lasheader->point_data_format != 4) && (lasheader->point_data_format != 5) && (lasheader->point_data_format != 9) && (lasheader->point_data_format != 10)) return 0;
  if (lasheader->wave_packet_descriptor == 0) return 0;
  if (get_path() == 0) return 0;
  LASwaveformreader* waveformreader = new LASwaveformreader();
  if ((lasheader->global_encoding & 2) && (lasheader->start_of_waveform_data_packet_record > lasheader->offset_to_point_data))
  {
    if (waveformreader->open(get_path(), lasheader->start_of_waveform_data_packet_record, lasheader->wave_packet_descriptor))
    {
      return waveformreader;
    }
  }
  else
  {
    if (waveformreader->open(get_path(), 0, lasheader->wave_packet_descriptor))
    {
      return waveformreader;
    }
  }
  delete waveformreader;
  return 0;
}

U32 LASreadOpener::get_format() const
{
  if (file_name)
  {
    if (strstr(file_name, ".laz") || strstr(file_name, ".LAZ") || strstr(file_name, ".las") || strstr(file_name, ".LAS"))
    {
      return 0;
    }
    else
    {
      fprintf(stderr, "ERROR: Unknown file extension '%s'.", file_name);
    }
  }
}

const char* LASreadOpener::get_path() const
{
  if (file_name)
    return file_name;
  if (file_name_number)
    return file_names[0];
  return 0;
}

const char* LASreadOpener::get_file_name() const
{
  const char* file_name_only = get_path();
  if (file_name_only)
  {
    int len = strlen(file_name_only);
    while ((len >= 0) && (file_name_only[len] != DIRECTORY_SLASH))
    {
      len--;
    }
    if (len >= 0)
    {
      return (file_name_only + len + 1);
    }
    else
    {
      return file_name_only;
    }
  }
  return 0;
}

#ifdef _WIN32

#include <windows.h>

BOOL LASreadOpener::add_file_name(const char* file_name, BOOL unique)
{
  BOOL r = FALSE;
  HANDLE h;
  WIN32_FIND_DATA info;
  h = FindFirstFile(file_name, &info);
  if (h != INVALID_HANDLE_VALUE)
  {
    // find the path
    int len = strlen(file_name);
    while ((len > 0) && (file_name[len] != '\\') && (file_name[len] != '/')) len--;
    if (len)
    {
      len++;
      char full_file_name[512];
      strncpy(full_file_name, file_name, len);
	    do
	    {
        sprintf(&full_file_name[len], "%s", info.cFileName);
        if (add_file_name_single(full_file_name, unique)) r = TRUE;
  	  } while (FindNextFile(h, &info));
    }
    else
    {
      do
      {
        if (add_file_name_single(info.cFileName, unique)) r = TRUE;
  	  } while (FindNextFile(h, &info));
    }
	  FindClose(h);
  }
  return r;
}

#endif

#ifdef _WIN32
BOOL LASreadOpener::add_file_name_single(const char* file_name, BOOL unique)
#else
BOOL LASreadOpener::add_file_name(const char* file_name, BOOL unique)
#endif
{
  if (unique)
  {
    U32 i;
    for (i = 0; i < file_name_number; i++)
    {
      if (strcmp(file_names[i], file_name) == 0)
      {
        return FALSE;
      }
    }
  }
  if (file_name_number == file_name_allocated)
  {
    if (file_names)
    {
      file_name_allocated *= 2;
      file_names = (char**)realloc(file_names, sizeof(char*)*file_name_allocated);
    }
    else
    {
      file_name_allocated = 16;
      file_names = (char**)malloc(sizeof(char*)*file_name_allocated);
    }
    if (file_names == 0)
    {
      fprintf(stderr, "ERROR: alloc for file_names pointer array failed at %d\n", file_name_allocated);
    }
  }
  file_names[file_name_number] = strdup(file_name);
  file_name_number++;
  return TRUE;
}

U32 LASreadOpener::get_file_name_number() const
{
  return file_name_number;
}

LASreadOpener::LASreadOpener()
{
  io_ibuffer_size = 65536;
  file_names = 0;
  file_name = 0;
  file_name_number = 0;
  file_name_allocated = 0;
  file_name_current = 0;
}

LASreadOpener::~LASreadOpener()
{
  if (file_names)
  {
    U32 i;
    for (i = 0; i < file_name_number; i++) free(file_names[i]);
    free(file_names);
  }
}
