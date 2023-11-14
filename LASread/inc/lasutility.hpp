/*
===============================================================================

  FILE:  lasutility.hpp
  
  CONTENTS:
  
    Simple utilities in the LASread library needed for the ASPRS LASvalidator

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
  
  COPYRIGHT:
  
    (c) 2007-2013, martin isenburg, rapidlasso - tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
    27 April 2013 -- adapted from the LASzip library for the ASPRS LASvalidator
  
===============================================================================
*/
#ifndef LAS_UTILITY_HPP
#define LAS_UTILITY_HPP

#include "laspoint.hpp"
#include "lasdefinitions.hpp"

class LASinventory
{
public:
  BOOL is_active() const { return (first == FALSE); }; 
  I64 number_of_point_records;
  I64 number_of_points_by_return[16];
  I64 number_of_returns_of_given_pulse[16];
  I64 return_count_for_return_number[16][16];
  I32 max_X;
  I32 min_X;
  I32 max_Y;
  I32 min_Y;
  I32 max_Z;
  I32 min_Z;
  U16 min_intensity;
  U16 max_intensity;
  U16 min_point_source_ID;
  U16 max_point_source_ID;
  I8 min_scan_angle_rank;
  I8 max_scan_angle_rank;
  I16 min_scan_angle;
  I16 max_scan_angle;
  F64 max_gps_time;
  F64 min_gps_time;
  U16 max_R;
  U16 min_R;
  U16 max_G;
  U16 min_G;
  U16 max_B;
  U16 min_B;
  U16 max_I;
  U16 min_I;
  I64 xyz_fluff_10[3];
  I64 xyz_fluff_100[3];
  I64 xyz_fluff_1000[3];
  U32 wave_packet_index_mask[8];
  BOOL add(const LASpoint* point);
  BOOL has_fluff() const { return has_fluff(0) || has_fluff(1) || has_fluff(2); };
  BOOL has_fluff(U32 i) const { return (number_of_point_records && (number_of_point_records == xyz_fluff_10[i])); };
  BOOL has_serious_fluff() const { return has_serious_fluff(0) || has_serious_fluff(1) || has_serious_fluff(2); };
  BOOL has_serious_fluff(U32 i) const { return (number_of_point_records && (number_of_point_records == xyz_fluff_100[i])); };
  BOOL has_very_serious_fluff() const { return has_very_serious_fluff(0) || has_very_serious_fluff(1) || has_very_serious_fluff(2); };
  BOOL has_very_serious_fluff(U32 i) const { return (number_of_point_records && (number_of_point_records == xyz_fluff_1000[i])); };
  BOOL has_wave_packet_index(const U8 index) const;
  LASinventory();
private:
  void set_wave_packet_index(const U8 index);
  BOOL first;
};

class LASLIB_DLL LASsummary
{
public:
  BOOL active() const { return (first == FALSE); };
  I64 number_of_point_records;
  I64 number_of_points_by_return[16];
  I64 number_of_returns[16];
  I64 classification[32];
  I64 extended_classification[256];
  I64 flagged_synthetic;
  I64 flagged_synthetic_classification[256];
  I64 flagged_keypoint;
  I64 flagged_keypoint_classification[256];
  I64 flagged_withheld;
  I64 flagged_withheld_classification[256];
  I64 flagged_extended_overlap;
  I64 flagged_extended_overlap_classification[256];
  LASpoint min;
  LASpoint max;
  U16 xyz_low_digits_10[3];
  U16 xyz_low_digits_100[3];
  U16 xyz_low_digits_1000[3];
  U16 xyz_low_digits_10000[3];
  I64 xyz_fluff_10[3];
  I64 xyz_fluff_100[3];
  I64 xyz_fluff_1000[3];
  I64 xyz_fluff_10000[3];
  BOOL add(const LASpoint* point);
  BOOL has_fluff() const { return has_fluff(0) || has_fluff(1) || has_fluff(2); };
  BOOL has_fluff(U32 i) const { return (number_of_point_records && ((min.get_XYZ())[i] != (max.get_XYZ())[i]) && (number_of_point_records == xyz_fluff_10[i])); };
  BOOL has_serious_fluff() const { return has_serious_fluff(0) || has_serious_fluff(1) || has_serious_fluff(2); };
  BOOL has_serious_fluff(U32 i) const { return (number_of_point_records && (number_of_point_records == xyz_fluff_100[i])); };
  BOOL has_very_serious_fluff() const { return has_very_serious_fluff(0) || has_very_serious_fluff(1) || has_very_serious_fluff(2); };
  BOOL has_very_serious_fluff(U32 i) const { return (number_of_point_records && (number_of_point_records == xyz_fluff_1000[i])); };
  BOOL has_extremely_serious_fluff() const { return has_extremely_serious_fluff(0) || has_extremely_serious_fluff(1) || has_extremely_serious_fluff(2); };
  BOOL has_extremely_serious_fluff(U32 i) const { return (number_of_point_records && (number_of_point_records == xyz_fluff_10000[i])); };
  LASsummary();
private:
  BOOL first;
};

#endif
