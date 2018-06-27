/****************************************************************************
 * main.cpp
 *
 * WFC converter for WiiFlow
 *
 * Wiimpathy 2018
 *
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <sys/time.h> 
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <algorithm>
#include <zlib.h>
#include "pngu.h"
#include "common.h"
#include "texture.h"

// Command line return codes
#define ARGV_OK              0
#define ARGV_UNKNOWN_OPTION  1
#define ARGV_PATH_NOT_FOUND  2

// Path containing the boxcovers.
char wiiflowpath[MAXPATHLEN];

// allow_texture_compression in WiiFlow.
bool compressTex = true;

// Toggle Wii/GC cache creation.
bool skipWii = true;

// Texture used for different image conversions.
STexture TexHandle;

// Append messages to this file.
char logfile[MAXPATHLEN];

// WFC file header.
struct SWFCHeader
{
#if 0
	u8 newFmt : 1;	// Was 0 in beta
	u8 full : 1;
	u8 cmpr : 1;
	u8 zipped : 1;
#endif
	u8 compress;
	u8 backCover : 1;
	u16 width;
	u16 height;
	u8 maxLOD;
	u16 backWidth;
	u16 backHeight;
	u8 backMaxLOD;
public:
	u32 getWidth(void) const { return width * 4; }
	u32 getHeight(void) const { return height * 4; }
	u32 getBackWidth(void) const { return backWidth * 4; }
	u32 getBackHeight(void) const { return backHeight * 4; }
	SWFCHeader(void)
	{
		memset(this, 0, sizeof *this);
	}
	SWFCHeader(const TexData &tex, bool f, bool z, const TexData &backTex = TexData())
	{
		// This byte gathers newFmt, full, cmpr and zipped.
		// Only cmpr (compress texture) can be toggled right now.
		compress = tex.format == GX_TF_CMPR ? 0xE0 : 0xC0;
		width = __builtin_bswap16(tex.width / 4);
		height = __builtin_bswap16(tex.height / 4);
		maxLOD = tex.maxLOD;
		backCover = !!backTex.data ? 1 : 0;
		backWidth = __builtin_bswap16(backTex.width / 4);
		backHeight = __builtin_bswap16(backTex.height / 4);
		backMaxLOD = backTex.maxLOD;
	}
};

/**
 * Browse the 'boxcovers' folder and create .wfc cache files 
 *
 * folder   : Current folder's path. Starts at 'boxcovers'.
 * compress : Enable/disable texture compression.
 * skip     : Enable/disable Wii and GameCube cache creation.
 */
void CreateCache(const char *folder, bool compress, bool skip)
{
	int i = 0, count = 0;
	u8 textureFmt = compress ? GX_TF_CMPR : GX_TF_RGB565;

	DIR *dir;
	struct dirent *entry;

	if (!(dir = opendir(folder)))
	{
		printf("\nError opening folder %s\n", folder);
		sleep(5);
		exit(0);
	}

	if (chdir(folder) == -1)
		printf("\nCan't access %s \n", folder);

	while((entry = readdir(dir)) != NULL) {
		char currentpath[MAXPATHLEN];

		struct stat statbuf;
		stat(entry->d_name,&statbuf);
		
		if(S_ISDIR(statbuf.st_mode)) // Folder : Open recursively
		{
			snprintf(currentpath, sizeof(currentpath), "%s%c%s", folder, separator, entry->d_name);

			// Skip parent and current directory
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;

			char *cachepath = (char*)malloc(MAXPATHLEN+1);
			sprintf(cachepath, "%s%ccache%c%s",  wiiflowpath, separator, separator, currentpath);
			printf("cachepath: %s\n", cachepath);
			
			// Create cache subdirectory if it doesn't exist
			DIR *dircache = NULL;
			dircache = opendir(cachepath);
			if (dircache) closedir(dircache);
#ifdef WIN32
			else mkdir(cachepath);
#else
			else mkdir(cachepath,S_IRWXU);
#endif
			free(cachepath);

			CreateCache(entry->d_name, compress, skip);
		}
		else  // Files : create cache files.
		{
			snprintf(currentpath, sizeof(currentpath), "%s",  entry->d_name);

			// Skips cache creation if we're in /wiiflow/boxcovers only
			if(skip && strcmp(folder, ".") == 0)
			{
				printf("Root, Wii cover : skip it!\n");
				continue;
			}
			else
			{
				// Get absolute boxcovers path
				char *fullpath = (char*)malloc(MAXPATHLEN+1);
				char cwd[MAXPATHLEN];
				char *coverfolder = NULL;

				if(getcwd(cwd, sizeof(cwd)) == NULL)
					printf("Can't get current directory!\n");
					
				coverfolder = strstr(cwd, "boxcovers");
				sprintf(fullpath, "%s%c%s%c",  wiiflowpath, separator, coverfolder, separator);
				strcat(fullpath, entry->d_name);

				// Strip extension
				char *ext = NULL;
				ext = strrchr(currentpath, '.');
				size_t ext_length = strlen(ext);
				size_t path_length = strlen(currentpath);
				currentpath[path_length - ext_length] = 0;

				// Cache file absolute path
				char wfcpath[MAXPATHLEN];
				sprintf(wfcpath, "%s%ccache%c%s%c%s.wfc", wiiflowpath, separator, separator, coverfolder+9, separator, currentpath);

				// Don't overwrite existing .wfc files.
				if(FileExist(wfcpath))
				{
					printf("%s already exists!\n", wfcpath);
					free(fullpath);
					continue;
				}
				else
				{
					// Open image and convert texture to desired format
					TexData tex;
					u32 bufSize = 0;
					bool m_compressCache = false; // FIXME : always false, not implemented.
					bool TexError = false;
					uLongf zBufferSize = 0;
					u8 *zBuffer = NULL;
					FILE *file = NULL;

					tex.thread = true;

					printf("\nConverting : %s\n", fullpath);
					TexError = TexHandle.fromImageFile(tex, fullpath, textureFmt, 32);

					bufSize = fixGX_GetTexBufferSize(tex.width, tex.height, tex.format, tex.maxLOD > 0 ? GX_TRUE : GX_FALSE, tex.maxLOD);
					zBufferSize = bufSize;
					zBuffer = tex.data;

					// No error detected, save the .wfc file.
					if(!TexError)
					{
						file = fopen (wfcpath, "wb");
						if(file != NULL && !TexError)
						{
							SWFCHeader header(tex, 1, m_compressCache);
							fwrite(&header, 1, sizeof header, file);
							fwrite(zBuffer, 1, zBufferSize, file);
							fclose(file);
							printf("%s saved.\n", wfcpath);
						}
					}
						// Free the texture and path
						TexHandle.Cleanup(tex);
						free(fullpath);
				}
			}
		}
	}
	if (chdir("..") == -1)
		printf("\nCan't access folder!\n");
		
	closedir(dir);
}

static void Usage(const char *exename, int error, const char *arg_error)
{
	ClearScreen();

	fprintf(stderr, "\nWFC converter %s (Wiimpathy 2018)\n", VERSION_STR);

	if(error == ARGV_UNKNOWN_OPTION)
	{
		fprintf(stderr, "\nError!!! Unknown option : %s\n", arg_error);
	}
	else if(error == ARGV_PATH_NOT_FOUND)
	{
		fprintf(stderr, "\nError!!! Wrong path: %s\n", arg_error);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: %s Path [-OPTIONS]\n", exename);
	fprintf(stderr, "\n");
	fprintf(stderr, "#Path#\n");
	fprintf(stderr, "  The path to the WiiFlow folder containing your boxcovers.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "#OPTIONS#\n");
	fprintf(stderr, "  -uncompress : Don't compress textures. Not recommended, files are large. It's disabled by default in WiiFlow. \n");
	fprintf(stderr, "  -wiicovers  : Create cache files for Wii and GameCube covers.\n");
	fprintf(stderr, "\n");
#ifdef WIN32
	fprintf(stderr, "Examples:\n");
	fprintf(stderr, "  %s e:\\WiiFlow\n", exename);
	fprintf(stderr, "  %s e:\\WiiFlow -wiicovers\n", exename);
#else
	fprintf(stderr, "Examples:\n");
	fprintf(stderr, "  %s /WiiFlow \n", exename);
	fprintf(stderr, "  %s /WiiFlow -wiicovers\n", exename);
#endif
	fprintf(stderr, "\n");
	exit(0);
}

static int HandleArguments(u32 argcount, char *argv[])
{
	u32  i;

	if(argcount < 2)
		Usage(argv[0], ARGV_OK, "");
 
	for (i = 1; i < argcount; i++)
	{
		if (argv[i][0] == '-')
		{
			if(i == 1)
				Usage(argv[0], ARGV_OK, "");

			char * option = argv[i];

			if (strcmp(option, "-uncompress") == 0)
			{
				printf("\ncompress  \n");
				compressTex = false;
			}
			else if (strcmp(option, "-wiicovers") == 0)
			{
				printf("\nwiicovers\n");
				skipWii = false;
			}
			else
			{
				Usage(argv[0], ARGV_UNKNOWN_OPTION, argv[i]);
			}
		}
		else
		{
			if(i != 1)
				Usage(argv[0], ARGV_UNKNOWN_OPTION, argv[i]);
		}
	}
}

int main (int argc, char *argv[])
{
	// WiiFlow boxcovers path
	char coverpath[MAXPATHLEN];

	// Check command lines options
	HandleArguments(argc, argv);

	sprintf(wiiflowpath, "%s",  argv[1]);
	sprintf(coverpath, "%s%cboxcovers",  wiiflowpath, separator);
	printf("\nWiiflow path: %s \n", wiiflowpath);

	// Exit if the 'boxcovers' folder is not found
	if(DirExist(coverpath))
	{
		if (chdir(coverpath) == -1)
			printf("\nCan't access %s \n", coverpath);
	}
	else
	{
		Usage(argv[0], ARGV_PATH_NOT_FOUND, argv[1]);
		exit(0);
	}

	// Create log file
	sprintf(logfile, "%s%clog_cachecreate.txt", wiiflowpath, separator);
	FILE *log;
	log = fopen(logfile, "wb");
	if (log == NULL)
	{
		ClearScreen();
		printf("Error! can't open log file %s\n", logfile);
		sleep(3);
	}
	fclose(log);

	// Timer to estimate duration
	time_t t0, t1;
	t0 = time(NULL);

	// Start browsing boxcovers and creating cache files
	CreateCache(".", compressTex, skipWii);

	// Get and Display elapsed time
	t1 = time(NULL);
	double elapsed = difftime(t1, t0);
	char Timetxt[MAXPATHLEN];

	format_elapsed_time(Timetxt, elapsed);
	printf("\nCache creation finished.\n");
	printf("It took about %s\n", Timetxt);

	// Append elapsed time to log file
	log = fopen(logfile, "a");
	if (log == NULL)
	{
		ClearScreen();
		printf("Error! can't open log file %s\n", logfile);
		sleep(3);
	}
	else
	{
		fprintf(log, "Cache generated in  %s\n", Timetxt);
	}
	fclose(log);

	return 0;
}

