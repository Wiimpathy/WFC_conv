/****************************************************************************
 * common.cpp
 *
 * WFC converter for WiiFlow
 *
 * Wiimpathy 2018
 *
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include "common.h"

#ifdef WIN32
char separator = '\\';
#else
char separator = '/';
#endif

bool FileExist(const char *path)
{
	FILE * f;
	f = fopen(path, "rb");
	if(f)
	{
		fclose(f);
		return true;
	}
	return false;
}

bool DirExist(const char *path)
{
	DIR *dir;
	dir = opendir(path);
	if(dir)
	{
		closedir(dir);
		return true;
	}
	return false;
}

void ClearScreen()
{
#ifdef WIN32
	system("cls");
#else
	printf("\x1b[2J");
	printf("\x1b[1;1H\n");
#endif
}

void format_elapsed_time(char *time_str, double elapsed)
{
	int h, m, s, ms;

	h = m = s = ms = 0;
	ms = elapsed * 1000; // promote the fractional part to milliseconds
	h = ms / 3600000;
	ms -= (h * 3600000);
	m = ms / 60000;
	ms -= (m * 60000);
	s = ms / 1000;
	ms -= (s * 1000);
	sprintf(time_str, "%02ih:%02im:%02is", h, m, s);
}

