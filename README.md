****************************************************************  
## lasvalidate

  This is a parallelized version of lasvalidate 200104 (LGPL) in
  C++ to validate or repair LAS/LAZ files on-the-fly in accordance
  with ASPRS LAS 1.0 to 1.4 specifications.

  This version runs serially by default, otherwise specify `-cores`
  and like the original version it does **not** work with buffered 
  tiles.

  For updates check the original website or the IRSS github page.
  Source code builds in Linux and Windows.

  Original version by Martin Isenburg.

  https://github.com/LASvalidator/
  https://github.com/IRSS-UBC

****************************************************************

## Build

Use .\bin\lasvalidate.exe for Windows and ./bin/lasvalidate 
for Linux.

To build from source code:
```commandline
mkdir lasvalidate
cd lasvalidate
git clone https://github.com/ttrotto/lasvalidate.git
mkdir build
cd build
cmake ..
make
```

## Usage
```commandline
C:\LAStools\bin lasvalidate.exe -h

Usage:
------
lasvalidate -i lidar.las
lasvalidate -i lidar.laz -no_CRS_fail
lasvalidate -i *.laz -cores 4 -v
lasvalidate -i *.laz -repair_unbuffered
lasvalidate -h
```

## Notes

Working on adding an internal check for buffered tiles.