****************************************************************

  lasvalidate:
  ------------

  This is a parallelized fork of version 200104 of lasvalidate (LGPL)
  in C++ to validate LAS/LAZ file conformation to the ASPRS LAS 1.0
  to 1.4 specifications.

  Source code builds compile in Linux and Windows.

  This version runs serially by default, otherwise specify `-cores`.

  For updates check the original website or the IRSS github page.

  Original version by Martin Isenburg.

  https://github.com/LASvalidator/
  https://github.com/IRSS-UBC

****************************************************************

## Build and run lasvalidate:

Use .\lasvalidate.exe for Windows execution and ./lasvalidate for Linux.

To build and compile the source code:
```commandline
mkdir LASvalidate
cd LASvalidate
git clone https://github.com/LASvalidator/LASread.git 
make
```

To run lasvalidate:
```commandline
C:\LAStools\bin lasvalidate.exe -i ..\data\*.las -oxml
C:\LAStools\bin lasvalidate.exe -i ..\data\*.las -o ..\data\summary.xml
C:\LAStools\bin lasvalidate.exe -i ..\data\*.laz -oxml -cores 4
```

## Usage
```commandline
C:\LAStools\bin lasvalidate.exe -h
This is version 1 of the parallelized fork of lasvalidate
originally developed by Martin Isenburg.

Supported Inputs:
-----------------
  -i lidar.las
  -i lidar1.las lidar2.las lidar3.las
  -i *.las
  -i flight0??.las flight1??.las
  -lof file_list.txt
  
Usage:
------
lasvalidate -i lidar.las
lasvalidate -i lidar.laz -cores 4
lasvalidate -i lidar.laz -no_CRS_fail
lasvalidate -v -i lidar.las -o report.xml
lasvalidate -v -i lidar.laz -oxml
lasvalidate -vv -i tile1.las tile2.las tile3.las -oxml
lasvalidate -i tile1.laz tile2.laz tile3.laz -o summary.xml
lasvalidate -i *.las -no_CRS_fail -o report.xml
lasvalidate -i *.laz -o summary.xml
lasvalidate -i *.laz -tile_size 1000 -o summary.xml
lasvalidate -i *.las -oxml
lasvalidate -i c:\data\lidar.las -oxml
lasvalidate -i ..\subfolder\*.las -o summary.xml
lasvalidate -v -i ..\..\flight\*.laz -o oxml
lasvalidate -h
```
