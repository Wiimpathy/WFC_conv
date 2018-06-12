# WFC_converter

WFC_converter is a command line tool to convert images to WiiFlow cache files. It scans all covers found in 'boxcovers' and save the .wfc files in the cache folder.
Only png are supported with the same constraints as in WiiFlow(max height < 1090 for example).

You may consult the log file 'log_cachecreate.txt' for any errors during the cache creation. Refer to the Return codes in pngu.h to see what the error number means.


## Usage

wfc_conv.exe Path [-OPTION]

#Path#

The path to the WiiFlow folder containing your boxcovers.


#OPTION#

-uncompress : Don't compress textures. Not recommended, files are large. It's disabled by default in WiiFlow.
  
-wiicovers  : Create cache files for Wii and GameCube covers.


Examples :

  wfc_conv.exe e:\WiiFlow
  
  wfc_conv.exe e:\WiiFlow -wiicovers


