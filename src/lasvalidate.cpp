/*
===============================================================================

  FILE:  lasvalidate.cpp
  
  CONTENTS:
  
    A tool to validate whether a LAS file conforms to the LAS specification

  PROGRAMMERS:
  
    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2016, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
     2 August 2015 -- not failing but warning if OCG WRT has intentional empty payload 
    12 April 2015 -- not failing but warning for certain empty VLR payloads 
    20 March 2015 -- fail on files containing zero point records
    26 January 2015 -- more useful reports if CRS always missing with '-no_CRS_fail'
     3 September 2013 -- made open source after the ASPRS LVS contract fiasko
     1 April 2013 -- on Easter Monday all-nighting in Perth airport for PER->SYD
  
===============================================================================
*/

#include <ctime>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "lasrepair.hpp"

#include "lasreadopener.hpp"
#include "xmlwriter.hpp"
#include "lascheck.hpp"

#define VALIDATE_VERSION  200104

#define VALIDATE_PASS     0x0000
#define VALIDATE_FAIL     0x0001
#define VALIDATE_WARNING  0x0002

static void byebye(int return_code, BOOL wait = TRUE)
{
  if (wait)
  {
    fprintf(stderr,"Process finished with return code %d\n", return_code);
    exit(return_code);
  }
  exit(return_code);
}

static void usage(int return_code, BOOL wait = FALSE)
{
  fprintf(stderr,"Usage:\n");
  fprintf(stderr,"------\n");
  fprintf(stderr,"lasvalidate -i lidar.las\n");
  fprintf(stderr,"lasvalidate -i lidar.laz -no_CRS_fail\n");
  fprintf(stderr,"lasvalidate -i *.laz -cores 4 -v \n");
  fprintf(stderr,"lasvalidate -i *.laz -repair_unbuffered\n");
  fprintf(stderr,"lasvalidate -h\n");
  byebye(return_code, wait);
}

static double taketime()
{
  return (double)(clock())/CLOCKS_PER_SEC;
}

#define LAS_VALIDATE_SUCCESS                    (0)  // Program successfully executed all phases
#define LAS_VALIDATE_UNKNOWN_ERROR             (-1)  // Program failed for an undeterminable reason
#define LAS_VALIDATE_WRONG_COMMAND_LINE_SYNTAX (-2)  // The command line does not conform to the syntax the LAS validator is expecting
#define LAS_VALIDATE_NO_INPUT_SPECIFIED        (-3)  // The command line does not specify any LAS or LAZ files as input
#define LAS_VALIDATE_INPUT_FILE_NOT_FOUND      (-4)  // The input file specified on the command line was not found

int main(int argc, char *argv[])
{
  int i;
  BOOL verbose = FALSE;
  F64 start_time = 0.0;
  F64 full_start_time = 0.0;
  BOOL no_CRS_fail = FALSE;
  U32 num_pass = 0;
  U32 num_fail = 0;
  U32 num_warning = 0;
  BOOL repair = FALSE;
  I32 cores = 1;

  LASreadOpener lasreadopener;

  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i],"-version") == 0)
    {
      fprintf(stderr, "\nlasvalidate %d with LASread (v %d.%d %d) and LAScheck (v %d.%d %d) by rapidlasso GmbH\n", VALIDATE_VERSION, LASREAD_VERSION_MAJOR, LASREAD_VERSION_MINOR, LASREAD_BUILD_DATE, LASCHECK_VERSION_MAJOR, LASCHECK_VERSION_MINOR, LASCHECK_BUILD_DATE);
      byebye(LAS_VALIDATE_SUCCESS, FALSE);
    }
    else if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0)
    {
      usage(LAS_VALIDATE_SUCCESS);
    }
    else if (strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"-verbose") == 0)
    {
      verbose = TRUE;
    }
    else if (strcmp(argv[i],"-i") == 0)
    {
      if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs at least 1 argument: file_name or wild_card\n", argv[i]);
        usage(LAS_VALIDATE_WRONG_COMMAND_LINE_SYNTAX);
      }
      i+=1;
      do
      {
        lasreadopener.add_file_name(argv[i]);
        i+=1;
      } while (i < argc && *argv[i] != '-');
      i-=1;
    }
    else if (strcmp(argv[i],"-no_CRS_fail") == 0)
    {
      no_CRS_fail = TRUE;
    }
    else if (strcmp(argv[i],"-cores") == 0)
    {
       if ((i+1) >= argc)
      {
        fprintf(stderr,"ERROR: '%s' needs at least 1 argument: cores\n", argv[i]);
        usage(LAS_VALIDATE_WRONG_COMMAND_LINE_SYNTAX);
      }
      i++;
      cores = stoi(argv[i]);
    }
    else if (strcmp(argv[i],"-repair_unbuffered") == 0)
    {
      repair = TRUE;
    }
    else
    {
      fprintf(stderr, "ERROR: cannot understand argument '%s'\n", argv[i]);
      usage(LAS_VALIDATE_WRONG_COMMAND_LINE_SYNTAX);
    }
  }

  // in verbose mode we measure the total time

  if (verbose) full_start_time = taketime();

  // check input

  if (!lasreadopener.is_active())
  {
    fprintf(stderr,"ERROR: no input specified\n");
    byebye(LAS_VALIDATE_NO_INPUT_SPECIFIED);
  }

  // accumulated pass

  U32 total_pass = VALIDATE_PASS;

  // loop over files

  for (i = 0; i < lasreadopener.get_file_name_number(); i++)
  {
    // in very verbose mode we measure the time for each file

    if (verbose) start_time = taketime();

    // open lasreader

    LASreader* lasreader = lasreadopener.open(i);
    if (!lasreader)
    {
      fprintf(stderr, "ERROR: could not open lasreader\n");
      byebye(LAS_VALIDATE_INPUT_FILE_NOT_FOUND);
    }

    // get a pointer to the header
    LASheader* lasheader = &lasreader->header;

    CHAR crsdescription[512];
    strcpy(crsdescription, "not valid or not specified");

    if (!lasheader->fails)
    {
      // header was loaded. now parse and check.

      LAScheck lascheck(lasheader);

      while (lasreader->read_point())
      {
        lascheck.parse(&lasreader->point);
      }

      // check header and points and get CRS description

      lascheck.check(lasheader, crsdescription, no_CRS_fail);
    }

    // report the verdict

    U32 pass = (lasheader->fails ? VALIDATE_FAIL : VALIDATE_PASS);
    if (lasheader->warnings) pass |= VALIDATE_WARNING;

    if (pass != VALIDATE_PASS)
    {
      BOOL success;
      total_pass |= pass;
      if (pass & VALIDATE_FAIL)
      {
        if (repair)
        {
          LASrepair lasrepair;
          success = lasrepair.repair_header(lasreader, lasreadopener.get_file_name());
        }
        if (success == 0) {num_fail++;};
      }
      else
      {
        if (repair)
        {
          LASrepair lasrepair;
          success = lasrepair.repair_header(lasreader, lasreadopener.get_file_name());
        }
        if (success == 0) {num_warning++;};
      }
    }
    else
    {
      num_pass++;
    }

    lasreader->close();
    delete lasreader;

    // in verbose mode we report the time for each file

    if (verbose)
    {
      fprintf(stdout,"done with '%s'. took %.2f sec. : %s\n", lasreadopener.get_file_name(), taketime()-start_time, (pass == VALIDATE_PASS ? "pass" : ((pass & VALIDATE_FAIL) ? "fail" : "warning")));
      start_time = taketime();
    }
  }

  // in verbose mode we report the total time

  if (verbose && (lasreadopener.get_file_name_number() > 1))
  {
    fprintf(stdout,"done. total time %.2f sec. total %s (pass=%d, warning=%d, fail=%d)\n", taketime()-full_start_time, (total_pass == 0 ? "pass" : ((total_pass & 1) ? "fail" : "warning")), num_pass, num_warning, num_fail);
  }

  byebye(LAS_VALIDATE_SUCCESS, FALSE);

  return 0;
}
