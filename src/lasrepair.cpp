/*
===============================================================================

  FILE:  lasrepair.hpp

  CONTENTS:

    A set of functions taken from lasinfo -repair for seamless integration
    with lasvalidate

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    tommaso.trotto@ubc.ca - IRSS UBC

  COPYRIGHT:

    (c) 2023 tommaso trotto, IRSS UBC

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

     November 5 2023 -- Added to repo

===============================================================================
*/

#include "lasrepair.hpp"
#include "lasreadopener.hpp"
#include "lasutility.hpp"
#include "lasheader.hpp"
//#include "lasvlr.hpp"

static const char* LASpointClassification [32] = {
  "never classified",
  "unclassified",
  "ground",
  "low vegetation",
  "medium vegetation",
  "high vegetation",
  "building",
  "noise",
  "keypoint",
  "water",
  "rail",
  "road surface",
  "overlap",
  "wire guard",
  "wire conductor",
  "tower",
  "wire connector",
  "bridge deck",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition",
  "Reserved for ASPRS Definition"
};

BOOL LASrepair::repair_header(LASreader* lasreader, const char* file_name)
{
    // declare common types
    FILE* file_out = 0;
    BOOL no_warnings = TRUE;
    BOOL no_returns = TRUE;
    BOOL no_min_max = TRUE;
    repair_me = TRUE;

    // open file and header

    FILE* file = fopen(file_name, "rb+");
    LASsummary lassummary;
    LASheader* lasheader = &lasreader->header;

//    // check if LAS is buffered
//    if (lasheader->vlr_lastiling->buffer)
//    {
//        LASquadtree lasquadtree;
//        lasquadtree.subtiling_setup(lasheader->vlr_lastiling->min_x, lasheader->vlr_lastiling->max_x, lasheader->vlr_lastiling->min_y, lasheader->vlr_lastiling->max_y, lasheader->vlr_lastiling->level, lasheader->vlr_lastiling->level_index, 0);
//        F32 min[2], max[2];
//        lasquadtree.get_cell_bounding_box(lasheader->vlr_lastiling->level_index, min, max);
//        F32 buffer = 0.0f;
//        buffer = (F32)(min[0] - lasheader->min_x);
//        if ((F32)(min[1] - lasheader->min_y) > buffer) buffer = (F32)(min[1] - lasheader->min_y);
//        if ((F32)(lasheader->max_x - max[0]) > buffer) buffer = (F32)(lasheader->max_x - max[0]);
//        if ((F32)(lasheader->max_y - max[1]) > buffer) buffer = (F32)(lasheader->max_y - max[1]);
//
//        if (buffer > 0.0f)
//        {
//            fprintf(stderr, "ERROR: cannot repair header of buffered input\n");
//            repair_me = false;
//        }
//    }
//    for (int i = 1; i < 16; i++) {
//        fprintf(stdout, "lassummary test: %lld", lasheader->number_of_points_by_return[i]);
//    }

    if (lasreadopener.get_format() != 0)
    {
      fprintf(stderr, "ERROR: can only repair header for LAS or LAZ files\n");
      repair_me = false;
    }

      // check number_of_point_records

      fprintf(stdout, "format : %d\n", lasheader->point_data_format);
      fprintf(stdout, "summary : %lld\n", lassummary.number_of_point_records);
      fprintf(stdout, "summary_header : %lld\n", lasheader->number_of_point_records);
      fprintf(stdout, "legacy : %u\n", lasheader->legacy_number_of_point_records);
      fprintf(stdout, "version : %u\n", lasheader->version_minor);

      // get unique value for all LAS versions

      U64 local_number_of_point_records;
      if (lasheader->version_minor < 4)
      {
        local_number_of_point_records = (U64)lasheader->legacy_number_of_point_records;
      }
      else if (lasheader->version_minor >= 4)
      {
        local_number_of_point_records = (U64)lasheader->number_of_point_records;
      }
      else
      {
        fprintf(stderr, "ERROR: cannot read number_of_point_records from header in '%s", file_name);
        return FALSE;
      }

      // start checks

      if ((lasheader->point_data_format < 6) && (lassummary.number_of_point_records != local_number_of_point_records))
      {
        if (repair_me)
        {
            fprintf(stdout, "number_of_point_records: %lld", lassummary.number_of_point_records);
          if (lassummary.number_of_point_records <= U32_MAX)
          {
            U32 number_of_point_records = (U32)lassummary.number_of_point_records;;
            fseek(file, 107, SEEK_SET);
            fwrite(&number_of_point_records, sizeof(U32), 1, file);
            if (file_out)
            {
              fprintf(file_out, "WARNING: real number of point records (%u) is different from header entry (%u). it was repaired. \n", number_of_point_records, lasheader->number_of_point_records);
            }
          }
          else if (lasheader->version_minor < 4)
          {
            if (file_out)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: real number of point records (%I64d) exceeds 4,294,967,295. cannot repair. too big.\n", lassummary.number_of_point_records);
#else
              fprintf(file_out, "WARNING: real number of point records (%lld) exceeds 4,294,967,295. cannot repair. too big.\n", lassummary.number_of_point_records);
#endif
            }
          }
          else if (local_number_of_point_records != 0)
          {
            U32 number_of_point_records = 0;
            fseek(file, 107, SEEK_SET);
            fwrite(&number_of_point_records, sizeof(U32), 1, file);
            if (file_out)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: real number of point records (%I64d) exceeds 4,294,967,295. but header entry is %u instead zero. it was repaired.\n", lassummary.number_of_point_records, lasheader->number_of_point_records);
#else
              fprintf(file_out, "WARNING: real number of point records (%lld) exceeds 4,294,967,295. but header entry is %u instead zero. it was repaired.\n", lassummary.number_of_point_records, lasheader->number_of_point_records);
#endif
            }
          }
          else
          {
            if (file_out)
            {
              fprintf(file_out, "number of point records in header is correct.\n");
            }
          }
        }
        else
        {
          if (!no_warnings && file_out)
          {
            if (lassummary.number_of_point_records <= U32_MAX)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: real number of point records (%I64d) is different from header entry (%u).\n", lassummary.number_of_point_records, lasheader->number_of_point_records);
#else
              fprintf(file_out, "WARNING: real number of point records (%lld) is different from header entry (%u).\n", lassummary.number_of_point_records, lasheader->number_of_point_records);
#endif
            }
            else if (lasheader->version_minor < 4)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: real number of point records (%I64d) exceeds 4,294,967,295.\n", lassummary.number_of_point_records);
#else
              fprintf(file_out, "WARNING: real number of point records (%lld) exceeds 4,294,967,295.\n", lassummary.number_of_point_records);
#endif
            }
            else if (local_number_of_point_records != 0)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: real number of point records (%I64d) exceeds 4,294,967,295. but header entry is %u instead of zero.\n", lassummary.number_of_point_records, lasheader->number_of_point_records);
#else
              fprintf(file_out, "WARNING: real number of point records (%lld) exceeds 4,294,967,295. but header entry is %u instead of zero.\n", lassummary.number_of_point_records, lasheader->number_of_point_records);
#endif
            }
          }
        }
      }
      else if ((lasheader->point_data_format >= 6) && (lasheader->number_of_point_records != 0))
      {
        if (repair_me)
        {
          U32 number_of_point_records = 0;
          fseek(file, 107, SEEK_SET);
          fwrite(&number_of_point_records, sizeof(U32), 1, file);
        }
        if (!no_warnings && file_out)
        {
          fprintf(file_out, "WARNING: point type is %d but (legacy) number of point records in header is %u instead zero.%s\n", lasheader->point_data_format, lasheader->number_of_point_records, (repair_me ? "it was repaired." : ""));
        }
      }
      else
      {
        if (repair_me)
        {
          if (file_out)
          {
            fprintf(file_out, "number of point records in header is correct.\n");
          }
        }
      }

      // check extended_number_of_point_records

      if (lasheader->version_minor > 3)
      {
        if (lassummary.number_of_point_records != (I64)lasheader->extended_number_of_point_records)
        {
          if (repair_me)
          {
            I64 extended_number_of_point_records = lassummary.number_of_point_records;
            fseek(file, 235 + 12, SEEK_SET);
            fwrite(&extended_number_of_point_records, sizeof(I64), 1, file);
          }
          if (!no_warnings && file_out)
          {
#ifdef _WIN32
            fprintf(file_out, "WARNING: real number of point records (%I64d) is different from extended header entry (%I64d).%s\n", lassummary.number_of_point_records, lasheader->extended_number_of_point_records, (repair_me ? " it was repaired." : ""));
#else
            fprintf(file_out, "WARNING: real number of point records (%lld) is different from extended header entry (%lld).%s\n", lassummary.number_of_point_records, lasheader->extended_number_of_point_records, (repair_me ? " it was repaired." : ""));
#endif
          }
        }
        else
        {
          if (repair_me)
          {
            if (file_out)
            {
              fprintf(file_out, "extended number of point records in header is correct.\n");
            }
          }
        }
      }

      // check number_of_points_by_return[5]

      bool was_set = false;
      for (int i = 1; i < 6; i++) if (lasheader->number_of_points_by_return[i-1]) was_set = true;

      bool wrong_entry = false;

      U32 number_of_points_by_return[5];
      for (int i = 1; i < 6; i++)
      {
        if ((lasheader->point_data_format < 6) && ((I64)(lasheader->number_of_points_by_return[i-1]) != lassummary.number_of_points_by_return[i]))
        {
          if (lassummary.number_of_points_by_return[i] <= U32_MAX)
          {
            number_of_points_by_return[i-1] = (U32)lassummary.number_of_points_by_return[i];
            wrong_entry = true;
            if (!no_warnings && file_out)
            {
              if (was_set)
              {
                fprintf(file_out, "WARNING: for return %d real number of points by return (%u) is different from header entry (%u).%s\n", i, number_of_points_by_return[i-1], lasheader->number_of_points_by_return[i-1], (repair_me ? " it was repaired." : ""));
              }
              else
              {
                fprintf(file_out, "WARNING: for return %d real number of points by return is %u but header entry was not set.%s\n", i, number_of_points_by_return[i-1], (repair_me ? " it was repaired." : ""));
              }
            }
          }
          else if (lasheader->version_minor < 4)
          {
            if (!no_warnings && file_out)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: for return %d real number of points by return (%I64d) exceeds 4,294,967,295.%s\n", i, lassummary.number_of_points_by_return[i], (repair_me ? " cannot repair. too big." : ""));
#else
              fprintf(file_out, "WARNING: for return %d real number of points by return (%lld) exceeds 4,294,967,295.%s\n", i, lassummary.number_of_points_by_return[i], (repair_me ? " cannot repair. too big." : ""));
#endif
            }
          }
          else if (lasheader->number_of_points_by_return[i-1] != 0)
          {
            number_of_points_by_return[i-1] = 0;
            wrong_entry = true;
            if (!no_warnings && file_out)
            {
#ifdef _WIN32
              fprintf(file_out, "WARNING: for return %d real number of points by return (%I64d) exceeds 4,294,967,295. but header entry is %u instead zero.%s\n", i, lassummary.number_of_points_by_return[i], lasheader->number_of_points_by_return[i-1], (repair_me ? " it was repaired." : ""));
#else
              fprintf(file_out, "WARNING: for return %d real number of points by return (%lld) exceeds 4,294,967,295. but header entry is %u instead zero.%s\n", i, lassummary.number_of_points_by_return[i], lasheader->number_of_points_by_return[i-1], (repair_me ? " it was repaired." : ""));
#endif
            }
          }
          else
          {
            number_of_points_by_return[i-1] = 0;
          }
        }
        else if ((lasheader->point_data_format >= 6) && (lasheader->number_of_points_by_return[i-1] != 0))
        {
          number_of_points_by_return[i-1] = 0;
          wrong_entry = true;
          if (!no_warnings && file_out)
          {
            fprintf(file_out, "WARNING: point type is %d but (legacy) number of points by return [%d] in header is %u instead zero.%s\n", lasheader->point_data_format, i, lasheader->number_of_points_by_return[i-1], (repair_me ? "it was repaired." : ""));
          }
        }
        else
        {
          number_of_points_by_return[i-1] = (U32)lassummary.number_of_points_by_return[i];
        }
      }

      if (repair_me)
      {
        if (wrong_entry)
        {
          fseek(file, 111, SEEK_SET);
          fwrite(&(number_of_points_by_return[0]), sizeof(U32), 5, file);
        }
        else if (file_out)
        {
          fprintf(file_out, "number of points by return in header is correct.\n");
        }
      }

      // check extended_number_of_points_by_return[15]

      if (lasheader->version_minor > 3)
      {
        bool was_set = false;
        for (int i = 1; i < 15; i++) if (lasheader->extended_number_of_points_by_return[i-1]) was_set = true;

        bool wrong_entry = false;

        I64 extended_number_of_points_by_return[15];

        for (int i = 1; i < 16; i++)
        {
          extended_number_of_points_by_return[i-1] = lassummary.number_of_points_by_return[i];
          if ((I64)lasheader->extended_number_of_points_by_return[i-1] != lassummary.number_of_points_by_return[i])
          {
            wrong_entry = true;
            if (!no_warnings && file_out)
            {
              if (was_set)
              {
#ifdef _WIN32
                fprintf(file_out, "WARNING: real extended number of points by return [%d] is %I64d - different from header entry %I64d.%s\n", i, lassummary.number_of_points_by_return[i], lasheader->extended_number_of_points_by_return[i-1], (repair_me ? " it was repaired." : ""));
#else
                fprintf(file_out, "WARNING: real extended number of points by return [%d] is %lld - different from header entry %lld.%s\n", i, lassummary.number_of_points_by_return[i], lasheader->extended_number_of_points_by_return[i-1], (repair_me ? " it was repaired." : ""));
#endif
              }
              else
              {
#ifdef _WIN32
                fprintf(file_out, "WARNING: real extended number of points by return [%d] is %I64d but header entry was not set.%s\n", i, lassummary.number_of_points_by_return[i], (repair_me ? " it was repaired." : ""));
#else
                fprintf(file_out, "WARNING: real extended number of points by return [%d] is %lld but header entry was not set.%s\n", i, lassummary.number_of_points_by_return[i], (repair_me ? " it was repaired." : ""));
#endif
              }
            }
          }
        }

        if (repair_me)
        {
          if (wrong_entry)
          {
            fseek(file, 235 + 20, SEEK_SET);
            fwrite(&(extended_number_of_points_by_return[0]), sizeof(I64), 15, file);
          }
          else if (file_out)
          {
            fprintf(file_out, "extended number of points by return in header is correct.\n");
          }
        }
      }

      if (!no_warnings && file_out && !no_returns)
      {
#ifdef _WIN32
        if (lassummary.number_of_points_by_return[0]) fprintf(file_out, "WARNING: there %s %I64d point%s with return number 0\n", (lassummary.number_of_points_by_return[0] > 1 ? "are" : "is"), lassummary.number_of_points_by_return[0], (lassummary.number_of_points_by_return[0] > 1 ? "s" : ""));
        if (lasheader->version_minor < 4)
        {
          if (lassummary.number_of_points_by_return[6]) fprintf(file_out, "WARNING: there %s %I64d point%s with return number 6\n", (lassummary.number_of_points_by_return[6] > 1 ? "are" : "is"), lassummary.number_of_points_by_return[6], (lassummary.number_of_points_by_return[6] > 1 ? "s" : ""));
          if (lassummary.number_of_points_by_return[7]) fprintf(file_out, "WARNING: there %s %I64d point%s with return number 7\n", (lassummary.number_of_points_by_return[7] > 1 ? "are" : "is"), lassummary.number_of_points_by_return[7], (lassummary.number_of_points_by_return[7] > 1 ? "s" : ""));
        }
#else
        if (lassummary.number_of_points_by_return[0]) fprintf(file_out, "WARNING: there %s %lld point%s with return number 0\n", (lassummary.number_of_points_by_return[0] > 1 ? "are" : "is"), lassummary.number_of_points_by_return[0], (lassummary.number_of_points_by_return[0] > 1 ? "s" : ""));
        if (lasheader->version_minor < 4)
        {
          if (lassummary.number_of_points_by_return[6]) fprintf(file_out, "WARNING: there %s %lld point%s with return number 6\n", (lassummary.number_of_points_by_return[6] > 1 ? "are" : "is"), lassummary.number_of_points_by_return[6], (lassummary.number_of_points_by_return[6] > 1 ? "s" : ""));
          if (lassummary.number_of_points_by_return[7]) fprintf(file_out, "WARNING: there %s %lld point%s with return number 7\n", (lassummary.number_of_points_by_return[7] > 1 ? "are" : "is"), lassummary.number_of_points_by_return[7], (lassummary.number_of_points_by_return[7] > 1 ? "s" : ""));
        }
#endif

        wrong_entry = false;

        if (lasheader->version_minor > 3)
        {
          for (int i = 1; i < 16; i++) if (lassummary.number_of_returns[i]) wrong_entry = true;
          if (wrong_entry)
          {
           fprintf(file_out, "overview over extended number of returns of given pulse:");
#ifdef _WIN32
            for (i = 1; i < 16; i++) fprintf(file_out, " %I64d", lassummary.number_of_returns[i]);
#else
            for (int i = 1; i < 16; i++) fprintf(file_out, " %lld", lassummary.number_of_returns[i]);
#endif
            fprintf(file_out, "\n");
          }
        }
        else
        {
          for (int i = 1; i < 8; i++) if (lassummary.number_of_returns[i]) wrong_entry = true;
          if (wrong_entry)
          {
           fprintf(file_out, "overview over number of returns of given pulse:");
#ifdef _WIN32
            for (i = 1; i < 8; i++) fprintf(file_out, " %I64d", lassummary.number_of_returns[i]);
#else
            for (int i = 1; i < 8; i++) fprintf(file_out, " %lld", lassummary.number_of_returns[i]);
#endif
            fprintf(file_out, "\n");
          }
        }

#ifdef _WIN32
        if (lassummary.number_of_returns[0]) fprintf(file_out, "WARNING: there are %I64d points with a number of returns of given pulse of 0\n", lassummary.number_of_returns[0]);
#else
        if (lassummary.number_of_returns[0]) fprintf(file_out, "WARNING: there are %lld points with a number of returns of given pulse of 0\n", lassummary.number_of_returns[0]);
#endif

      if (file_out && !no_min_max)
      {
        wrong_entry = false;
        for (int i = 0; i < 32; i++) if (lassummary.classification[i]) wrong_entry = true;
        if (lassummary.flagged_synthetic || lassummary.flagged_keypoint || lassummary.flagged_withheld) wrong_entry = true;

        if (wrong_entry)
        {
          fprintf(file_out, "histogram of classification of points:\n");
#ifdef _WIN32
          for (i = 0; i < 32; i++) if (lassummary.classification[i]) fprintf(file_out, " %15I64d  %s (%u)\n", lassummary.classification[i], LASpointClassification[i], i);
          if (lassummary.flagged_synthetic)
          {
            fprintf(file_out, " +-> flagged as synthetic: %I64d\n", lassummary.flagged_synthetic);
            for (i = 0; i < 32; i++) if (lassummary.flagged_synthetic_classification[i]) fprintf(file_out, "  +---> %15I64d of those are %s (%u)\n", lassummary.flagged_synthetic_classification[i], LASpointClassification[i], i);
            for (i = 32; i < 256; i++) if (lassummary.flagged_synthetic_classification[i]) fprintf(file_out, "  +---> %15I64d  of those are classification (%u)\n", lassummary.flagged_synthetic_classification[i], i);
          }
          if (lassummary.flagged_keypoint)
          {
            fprintf(file_out,  " +-> flagged as keypoints: %I64d\n", lassummary.flagged_keypoint);
            for (i = 0; i < 32; i++) if (lassummary.flagged_keypoint_classification[i]) fprintf(file_out, "  +---> %15I64d of those are %s (%u)\n", lassummary.flagged_keypoint_classification[i], LASpointClassification[i], i);
            for (i = 32; i < 256; i++) if (lassummary.flagged_keypoint_classification[i]) fprintf(file_out, "  +---> %15I64d  of those are classification (%u)\n", lassummary.flagged_keypoint_classification[i], i);
          }
          if (lassummary.flagged_withheld)
          {
            fprintf(file_out,  " +-> flagged as withheld:  %I64d\n", lassummary.flagged_withheld);
            for (i = 0; i < 32; i++) if (lassummary.flagged_withheld_classification[i]) fprintf(file_out, "  +---> %15I64d of those are %s (%u)\n", lassummary.flagged_withheld_classification[i], LASpointClassification[i], i);
            for (i = 32; i < 256; i++) if (lassummary.flagged_withheld_classification[i]) fprintf(file_out, "  +---> %15I64d  of those are classification (%u)\n", lassummary.flagged_withheld_classification[i], i);
          }
#else
          for (int i = 0; i < 32; i++) if (lassummary.classification[i]) fprintf(file_out, " %15lld  %s (%u)\n", lassummary.classification[i], LASpointClassification[i], i);
          if (lassummary.flagged_synthetic)
          {
            fprintf(file_out, " +-> flagged as synthetic: %lld\n", lassummary.flagged_synthetic);
            for (int i = 0; i < 32; i++) if (lassummary.flagged_synthetic_classification[i]) fprintf(file_out, "  +---> %15lld of those are %s (%u)\n", lassummary.flagged_synthetic_classification[i], LASpointClassification[i], i);
            for (int i = 32; i < 256; i++) if (lassummary.flagged_synthetic_classification[i]) fprintf(file_out, "  +---> %15lld  of those are classification (%u)\n", lassummary.flagged_synthetic_classification[i], i);
          }
          if (lassummary.flagged_keypoint)
          {
            fprintf(file_out,  " +-> flagged as keypoints: %lld\n", lassummary.flagged_keypoint);
            for (int i = 0; i < 32; i++) if (lassummary.flagged_keypoint_classification[i]) fprintf(file_out, "  +---> %15lld of those are %s (%u)\n", lassummary.flagged_keypoint_classification[i], LASpointClassification[i], i);
            for (int i = 32; i < 256; i++) if (lassummary.flagged_keypoint_classification[i]) fprintf(file_out, "  +---> %15lld  of those are classification (%u)\n", lassummary.flagged_keypoint_classification[i], i);
          }
          if (lassummary.flagged_withheld)
          {
            fprintf(file_out,  " +-> flagged as withheld:  %lld\n", lassummary.flagged_withheld);
            for (int i = 0; i < 32; i++) if (lassummary.flagged_withheld_classification[i]) fprintf(file_out, "  +---> %15lld of those are %s (%u)\n", lassummary.flagged_withheld_classification[i], LASpointClassification[i], i);
            for (int i = 32; i < 256; i++) if (lassummary.flagged_withheld_classification[i]) fprintf(file_out, "  +---> %15lld  of those are classification (%u)\n", lassummary.flagged_withheld_classification[i], i);
          }
#endif
        }

        if (lasreader->point.get_extended_point_type())
        {
#ifdef _WIN32
          if (lassummary.flagged_extended_overlap)
          {
            fprintf(file_out, " +-> flagged as extended overlap: %I64d\n", lassummary.flagged_extended_overlap);
            for (i = 0; i < 32; i++) if (lassummary.flagged_extended_overlap_classification[i]) fprintf(file_out, "  +---> %15I64d of those are %s (%u)\n", lassummary.flagged_extended_overlap_classification[i], LASpointClassification[i], i);
            for (i = 32; i < 256; i++) if (lassummary.flagged_extended_overlap_classification[i]) fprintf(file_out, "  +---> %15I64d  of those are classification (%u)\n", lassummary.flagged_extended_overlap_classification[i], i);
          }
#else
          if (lassummary.flagged_extended_overlap)
          {
            fprintf(file_out, " +-> flagged as extended overlap: %lld\n", lassummary.flagged_extended_overlap);
            for (int i = 0; i < 32; i++) if (lassummary.flagged_extended_overlap_classification[i]) fprintf(file_out, "  +---> %15lld of those are %s (%u)\n", lassummary.flagged_extended_overlap_classification[i], LASpointClassification[i], i);
            for (int i = 32; i < 256; i++) if (lassummary.flagged_extended_overlap_classification[i]) fprintf(file_out, "  +---> %15lld  of those are classification (%u)\n", lassummary.flagged_extended_overlap_classification[i], i);
          }
#endif

          wrong_entry = false;
          for (int i = 32; i < 256; i++) if (lassummary.extended_classification[i]) wrong_entry = true;

          if (wrong_entry)
          {
            fprintf(file_out, "histogram of extended classification of points:\n");
  #ifdef _WIN32
            for (i = 32; i < 256; i++) if (lassummary.extended_classification[i]) fprintf(file_out, " %15I64d  extended classification (%u)\n", lassummary.extended_classification[i], i);
  #else
            for (int i = 32; i < 256; i++) if (lassummary.extended_classification[i]) fprintf(file_out, " %15lld  extended classification (%u)\n", lassummary.extended_classification[i], i);
  #endif
          }
        }
      }

      double value;
      if (repair_me)
      {
        wrong_entry = false;
        if (lasheader->get_x(lassummary.max.get_X()) != lasheader->max_x) wrong_entry = true;
        if (lasheader->get_x(lassummary.min.get_X()) != lasheader->min_x) wrong_entry = true;
        if (lasheader->get_y(lassummary.max.get_Y()) != lasheader->max_y) wrong_entry = true;
        if (lasheader->get_y(lassummary.min.get_Y()) != lasheader->min_y) wrong_entry = true;
        if (lasheader->get_z(lassummary.max.get_Z()) != lasheader->max_z) wrong_entry = true;
        if (lasheader->get_z(lassummary.min.get_Z()) != lasheader->min_z) wrong_entry = true;
        if (wrong_entry)
        {
          fseek(file, 179, SEEK_SET);
          value = lasheader->get_x(lassummary.max.get_X()); fwrite(&value, sizeof(double), 1, file);
          value = lasheader->get_x(lassummary.min.get_X()); fwrite(&value, sizeof(double), 1, file);
          value = lasheader->get_y(lassummary.max.get_Y()); fwrite(&value, sizeof(double), 1, file);
          value = lasheader->get_y(lassummary.min.get_Y()); fwrite(&value, sizeof(double), 1, file);
          value = lasheader->get_z(lassummary.max.get_Z()); fwrite(&value, sizeof(double), 1, file);
          value = lasheader->get_z(lassummary.min.get_Z()); fwrite(&value, sizeof(double), 1, file);
          if (file_out) fprintf(file_out, "bounding box was repaired.\n");
        }
        else
        {
          if (file_out) fprintf(file_out, "bounding box is correct.\n");
        }
      }
//      else
//      {
//        value = lasheader->get_x(lassummary.max.get_X());
//        if (value > enlarged_max_x)
//        {
//          if (!no_warnings && file_out) fprintf(file_out, "WARNING: real max x larger than header max x by %lf\n", value - lasheader->max_x);
//        }
//        value = lasheader->get_x(lassummary.min.get_X());
//        if (value < enlarged_min_x)
//        {
//          if (!no_warnings && file_out) fprintf(file_out, "WARNING: real min x smaller than header min x by %lf\n", lasheader->min_x - value);
//        }
//        value = lasheader->get_y(lassummary.max.get_Y());
//        if (value > enlarged_max_y)
//        {
//          if (!no_warnings && file_out) fprintf(file_out, "WARNING: real max y larger than header max y by %lf\n", value - lasheader->max_y);
//        }
//        value = lasheader->get_y(lassummary.min.get_Y());
//        if (value < enlarged_min_y)
//        {
//          if (!no_warnings && file_out) fprintf(file_out, "WARNING: real min y smaller than header min y by %lf\n", lasheader->min_y - value);
//        }
//        value = lasheader->get_z(lassummary.max.get_Z());
//        if (value > enlarged_max_z)
//        {
//          if (!no_warnings && file_out) fprintf(file_out, "WARNING: real max z larger than header max z by %lf\n", value - lasheader->max_z);
//        }
//        value = lasheader->get_z(lassummary.min.get_Z());
//        if (value < enlarged_min_z)
//        {
//          if (!no_warnings && file_out) fprintf(file_out, "WARNING: real min z smaller than header min z by %lf\n", lasheader->min_z - value);
//        }
//      }
    }

    if (file_out && (file_out != stdout) && (file_out != stderr)) fclose(file_out);
//    laswriteopener.set_file_name(0);

    delete lasreader;
    if (file) fclose(file);
}
