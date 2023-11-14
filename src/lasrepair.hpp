/*
===============================================================================

  FILE:  lasreapir.hpp

  CONTENTS:

    Contains the header definitions for LAS formats 1.0 to 1.4

      Version 1.4,   Nov 14, 2011.
      Version 1.3,   Oct 24, 2010.
      Version 1.2, April 29, 2008.
      Version 1.1, March 07, 2005.
      Version 1.0,   May 09, 2003

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2005-2013, martin isenburg, rapidlasso - tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

     2 August 2015 -- not failing but warning if OCG WRT has an empty payload
    12 April 2015 -- not failing but warning for certain empty VLR payloads
    27 April 2013 -- adapted from the LASlib library for the ASPRS LASvalidator

===============================================================================
*/

#ifndef LASVALIDATE_LASREPAIR_HPP
#define LASVALIDATE_LASREPAIR_HPP

#include "geoprojectionconverter.hpp"
#include "lasreader.hpp"
#include "lasreadopener.hpp"
#include "laszip_decompress_selective_v3.hpp"
#include "lasutility.hpp"
#include "laswriter.hpp"
#include "lasquadtree.hpp"
#include "lasvlrpayload.hpp"
#include "lasindex.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#ifdef _WIN32
#include <windows.h>
#endif

class LASrepair
{
public:
    BOOL repair_header(LASreader* lasreader, const char* file_name);
//    LASrepair();
//    ~LASrepair();
private:
    LASreadOpener lasreadopener;
    BOOL repair_me;
    char* file_name;
};

#endif //LASVALIDATE_LASREPAIR_HPP
