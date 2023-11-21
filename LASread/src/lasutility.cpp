/*
===============================================================================

  FILE:  lasutility.cpp
  
  CONTENTS:
  
    see corresponding header file
  
  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2013, martin isenburg, rapidlasso - fast tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    see corresponding header file
  
===============================================================================
*/
#include "lasutility.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

LASinventory::LASinventory()
{
  memset(this, 0, sizeof(LASinventory));
  first = TRUE;
}

LASsummary::LASsummary()
{
  U32 i;
  number_of_point_records = 0;
  for (i = 0; i < 16; i++) number_of_points_by_return[i] = 0;
  for (i = 0; i < 16; i++) number_of_returns[i] = 0;
  for (i = 0; i < 32; i++) classification[i] = 0;
  for (i = 0; i < 256; i++)
  {
    extended_classification[i] = 0;
    flagged_synthetic_classification[i] = 0;
    flagged_keypoint_classification[i] = 0;
    flagged_withheld_classification[i] = 0;
    flagged_extended_overlap_classification[i] = 0;
  }
  for (i = 0; i < 3; i++)
  {
    xyz_fluff_10[i] = 0;
    xyz_fluff_100[i] = 0;
    xyz_fluff_1000[i] = 0;
    xyz_fluff_10000[i] = 0;
  }
  flagged_synthetic = 0;
  flagged_keypoint = 0;
  flagged_withheld = 0;
  flagged_extended_overlap = 0;
  first = TRUE;
}

BOOL LASsummary::add(const LASpoint* point)
{
  number_of_point_records++;
  if (point->extended_point_type)
  {
    number_of_points_by_return[point->get_extended_return_number()]++;
    number_of_returns[point->get_extended_number_of_returns()]++;
    if (point->get_extended_classification() > 31)
    {
      extended_classification[point->get_extended_classification()]++;
    }
    else
    {
      classification[point->get_classification()]++;
    }
  }
  else
  {
    number_of_points_by_return[point->get_return_number()]++;
    classification[point->get_classification()]++;
    number_of_returns[point->get_number_of_returns()]++;
  }
  if (first)
  {
    // does the point have extra bytes
//    if (point->extra_bytes_number)
//    {
//      min.extra_bytes = new U8[point->extra_bytes_number];
//      min.extra_bytes_number = point->extra_bytes_number;
//      max.extra_bytes = new U8[point->extra_bytes_number];
//      max.extra_bytes_number = point->extra_bytes_number;
//    }
    // initialize min and max
    min = *point;
    max = *point;
    // initialize fluff detection
    xyz_low_digits_10[0] = (U16)(point->get_X()%10);
    xyz_low_digits_10[1] = (U16)(point->get_Y()%10);
    xyz_low_digits_10[2] = (U16)(point->get_Z()%10);
    xyz_low_digits_100[0] = (U16)(point->get_X()%100);
    xyz_low_digits_100[1] = (U16)(point->get_Y()%100);
    xyz_low_digits_100[2] = (U16)(point->get_Z()%100);
    xyz_low_digits_1000[0] = (U16)(point->get_X()%1000);
    xyz_low_digits_1000[1] = (U16)(point->get_Y()%1000);
    xyz_low_digits_1000[2] = (U16)(point->get_Z()%1000);
    xyz_low_digits_10000[0] = (U16)(point->get_X()%10000);
    xyz_low_digits_10000[1] = (U16)(point->get_Y()%10000);
    xyz_low_digits_10000[2] = (U16)(point->get_Z()%10000);
    first = FALSE;
  }
  else {
    if (point->get_X() < min.get_X()) min.set_X(point->get_X());
    else if (point->get_X() > max.get_X()) max.set_X(point->get_X());
    if (point->get_Y() < min.get_Y()) min.set_Y(point->get_Y());
    else if (point->get_Y() > max.get_Y()) max.set_Y(point->get_Y());
    if (point->get_Z() < min.get_Z()) min.set_Z(point->get_Z());
    else if (point->get_Z() > max.get_Z()) max.set_Z(point->get_Z());
    if (point->intensity < min.intensity) min.intensity = point->intensity;
    else if (point->intensity > max.intensity) max.intensity = point->intensity;
    if (point->edge_of_flight_line < min.edge_of_flight_line) min.edge_of_flight_line = point->edge_of_flight_line;
    else if (point->edge_of_flight_line > max.edge_of_flight_line) max.edge_of_flight_line = point->edge_of_flight_line;
    if (point->scan_direction_flag < min.scan_direction_flag) min.scan_direction_flag = point->scan_direction_flag;
    else if (point->scan_direction_flag > max.scan_direction_flag) max.scan_direction_flag = point->scan_direction_flag;
    if (point->number_of_returns < min.number_of_returns) min.number_of_returns = point->number_of_returns;
    else if (point->number_of_returns > max.number_of_returns) max.number_of_returns = point->number_of_returns;
    if (point->return_number < min.return_number) min.return_number = point->return_number;
    else if (point->return_number > max.return_number) max.return_number = point->return_number;
    if (point->classification < min.classification) min.classification = point->classification;
    else if (point->classification > max.classification) max.classification = point->classification;
    if (point->scan_angle_rank < min.scan_angle_rank) min.scan_angle_rank = point->scan_angle_rank;
    else if (point->scan_angle_rank > max.scan_angle_rank) max.scan_angle_rank = point->scan_angle_rank;
    if (point->user_data < min.user_data) min.user_data = point->user_data;
    else if (point->user_data > max.user_data) max.user_data = point->user_data;
    if (point->point_source_ID < min.point_source_ID) min.point_source_ID = point->point_source_ID;
    else if (point->point_source_ID > max.point_source_ID) max.point_source_ID = point->point_source_ID;
  }
//    if (point->extended_point_type)
//    {
//      if (point->extended_classification < min.extended_classification) min.extended_classification = point->extended_classification;
//      else if (point->extended_classification > max.extended_classification) max.extended_classification = point->extended_classification;
//      if (point->extended_return_number < min.extended_return_number) min.extended_return_number = point->extended_return_number;
//      else if (point->extended_return_number > max.extended_return_number) max.extended_return_number = point->extended_return_number;
//      if (point->extended_number_of_returns < min.extended_number_of_returns) min.extended_number_of_returns = point->extended_number_of_returns;
//      else if (point->extended_number_of_returns > max.extended_number_of_returns) max.extended_number_of_returns = point->extended_number_of_returns;
//      if (point->extended_scan_angle < min.extended_scan_angle) min.extended_scan_angle = point->extended_scan_angle;
//      else if (point->extended_scan_angle > max.extended_scan_angle) max.extended_scan_angle = point->extended_scan_angle;
//      if (point->extended_scanner_channel < min.extended_scanner_channel) min.extended_scanner_channel = point->extended_scanner_channel;
//      else if (point->extended_scanner_channel > max.extended_scanner_channel) max.extended_scanner_channel = point->extended_scanner_channel;
//    }
//    if (point->extra_bytes_number)
//    {
//      if (point->attributer)
//      {
//        min.attributer = point->attributer;
//        max.attributer = point->attributer;
//        I32 a;
//        for (a = 0; a < point->attributer->number_attributes; a++)
//        {
//          F64 value = point->get_attribute_as_float(a);
//          if (value < min.get_attribute_as_float(a))
//          {
//            min.set_attribute_as_float(a, value);
//          }
//          if (value > max.get_attribute_as_float(a))
//          {
//            max.set_attribute_as_float(a, value);
//          }
//        }
//        min.attributer = 0;
//        max.attributer = 0;
//      }
//    }
//  }
//  if (((U16)(point->get_X()%10)) == xyz_low_digits_10[0])
//  {
//    xyz_fluff_10[0]++;
//    if (((U16)(point->get_X()%100)) == xyz_low_digits_100[0])
//    {
//      xyz_fluff_100[0]++;
//      if (((U16)(point->get_X()%1000)) == xyz_low_digits_1000[0])
//      {
//        xyz_fluff_1000[0]++;
//        if (((U16)(point->get_X()%10000)) == xyz_low_digits_10000[0])
//        {
//          xyz_fluff_10000[0]++;
//        }
//      }
//    }
//  }
//  if (((U16)(point->get_Y()%10)) == xyz_low_digits_10[1])
//  {
//    xyz_fluff_10[1]++;
//    if (((U16)(point->get_Y()%100)) == xyz_low_digits_100[1])
//    {
//      xyz_fluff_100[1]++;
//      if (((U16)(point->get_Y()%1000)) == xyz_low_digits_1000[1])
//      {
//        xyz_fluff_1000[1]++;
//        if (((U16)(point->get_Y()%10000)) == xyz_low_digits_10000[1])
//        {
//          xyz_fluff_10000[1]++;
//        }
//      }
//    }
//  }
//  if (((U16)(point->get_Z()%10)) == xyz_low_digits_10[2])
//  {
//    xyz_fluff_10[2]++;
//    if (((U16)(point->get_Z()%100)) == xyz_low_digits_100[2])
//    {
//      xyz_fluff_100[2]++;
//      if (((U16)(point->get_Z()%1000)) == xyz_low_digits_1000[2])
//      {
//        xyz_fluff_1000[2]++;
//        if (((U16)(point->get_Z()%10000)) == xyz_low_digits_10000[2])
//        {
//          xyz_fluff_10000[2]++;
//        }
//      }
//    }
//  }
  return TRUE;
}
void LASinventory::set_wave_packet_index(const U8 index)
{
  wave_packet_index_mask[index/32] |= (1u << (index%32));
}

BOOL LASinventory::has_wave_packet_index(const U8 index) const
{
  return ((wave_packet_index_mask[index/32] & (1u << (index%32))) != 0);
}

BOOL LASinventory::add(const LASpoint* point)
{
  number_of_point_records++;
  number_of_points_by_return[point->get_return_number()]++;
  number_of_returns_of_given_pulse[point->get_number_of_returns_of_given_pulse()]++;
  return_count_for_return_number[point->get_number_of_returns_of_given_pulse()][point->get_return_number()]++;
  if (first)
  {
    min_X = max_X = point->get_X();
    min_Y = max_Y = point->get_Y();
    min_Z = max_Z = point->get_Z();
    min_intensity = max_intensity = point->get_intensity();
    min_point_source_ID = max_point_source_ID = point->get_point_source_ID();
    min_scan_angle_rank = max_scan_angle_rank = point->get_scan_angle_rank();
    min_scan_angle = max_scan_angle = point->get_scan_angle();
    min_gps_time = max_gps_time = point->get_gps_time();
    max_R = min_R = point->get_R();
    max_G = min_G = point->get_G();
    max_B = min_B = point->get_B();
    max_I = min_I = point->get_I();
    first = FALSE;
  }
  else
  {
    if (point->get_X() < min_X) min_X = point->get_X();
    else if (point->get_X() > max_X) max_X = point->get_X();
    if (point->get_Y() < min_Y) min_Y = point->get_Y();
    else if (point->get_Y() > max_Y) max_Y = point->get_Y();
    if (point->get_Z() < min_Z) min_Z = point->get_Z();
    else if (point->get_Z() > max_Z) max_Z = point->get_Z();
    if (point->get_intensity() < min_intensity) min_intensity = point->get_intensity();
    else if (point->get_intensity() > max_intensity) max_intensity = point->get_intensity();
    if (point->get_point_source_ID() < min_point_source_ID) min_point_source_ID = point->get_point_source_ID();
    else if (point->get_point_source_ID() > max_point_source_ID) max_point_source_ID = point->get_point_source_ID();
    if (point->get_scan_angle_rank() < min_scan_angle_rank) min_scan_angle_rank = point->get_scan_angle_rank();
    else if (point->get_scan_angle_rank() > max_scan_angle_rank) max_scan_angle_rank = point->get_scan_angle_rank();
    if (point->get_scan_angle() < min_scan_angle) min_scan_angle = point->get_scan_angle();
    else if (point->get_scan_angle() > max_scan_angle) max_scan_angle = point->get_scan_angle();
    if (point->get_gps_time() < min_gps_time) min_gps_time = point->get_gps_time();
    else if (point->get_gps_time() > max_gps_time) max_gps_time = point->get_gps_time();
    if (point->get_R() < min_R) min_R = point->get_R();
    else if (point->get_R() > max_R) max_R = point->get_R();
    if (point->get_G() < min_G) min_G = point->get_G();
    else if (point->get_G() > max_G) max_G = point->get_G();
    if (point->get_B() < min_B) min_B = point->get_B();
    else if (point->get_B() > max_B) max_B = point->get_B();
    if (point->get_I() < min_I) min_I = point->get_I();
    else if (point->get_I() > max_I) max_I = point->get_I();
  }
  if ((point->get_X()%10) == 0)
  {
    xyz_fluff_10[0]++;
    if ((point->get_X()%100) == 0)
    {
      xyz_fluff_100[0]++;
      if ((point->get_X()%1000) == 0)
      {
        xyz_fluff_1000[0]++;
      }
    }
  }
  if ((point->get_Y()%10) == 0)
  {
    xyz_fluff_10[1]++;
    if ((point->get_Y()%100) == 0)
    {
      xyz_fluff_100[1]++;
      if ((point->get_Y()%1000) == 0)
      {
        xyz_fluff_1000[1]++;
      }
    }
  }
  if ((point->get_Z()%10) == 0)
  {
    xyz_fluff_10[2]++;
    if ((point->get_Z()%100) == 0)
    {
      xyz_fluff_100[2]++;
      if ((point->get_Z()%1000) == 0)
      {
        xyz_fluff_1000[2]++;
      }
    }
  }
  if (point->has_wave_packet())
  {
    set_wave_packet_index(point->get_wave_packet_index());
  }
  return TRUE;
}
