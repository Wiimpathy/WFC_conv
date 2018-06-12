
#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

#define MAXPATHLEN 1024

// libogc gx defines
#define 	_GX_TF_CTF   0x20 
#define 	_GX_TF_ZTF   0x10
#define 	GX_TF_Z16   (0x3|_GX_TF_ZTF)
#define 	GX_TF_Z24X8   (0x6|_GX_TF_ZTF)
#define 	GX_TF_Z8   (0x1|_GX_TF_ZTF)
#define 	GX_CTF_A8   (0x7|_GX_TF_CTF)
#define 	GX_CTF_B8   (0xA|_GX_TF_CTF)
#define 	GX_CTF_G8   (0x9|_GX_TF_CTF) 
#define 	GX_CTF_GB8   (0xC|_GX_TF_CTF)
#define 	GX_CTF_R4   (0x0|_GX_TF_CTF)
#define 	GX_CTF_R8   (0x8|_GX_TF_CTF)
#define 	GX_CTF_RA4   (0x2|_GX_TF_CTF)
#define 	GX_CTF_RA8   (0x3|_GX_TF_CTF)
#define 	GX_CTF_RG8   (0xB|_GX_TF_CTF)
#define 	GX_CTF_YUVA8   (0x6|_GX_TF_CTF)
#define 	GX_CTF_Z16L   (0xC|_GX_TF_ZTF|_GX_TF_CTF)
#define 	GX_CTF_Z4   (0x0|_GX_TF_ZTF|_GX_TF_CTF)
#define 	GX_CTF_Z8L   (0xA|_GX_TF_ZTF|_GX_TF_CTF)
#define 	GX_CTF_Z8M   (0x9|_GX_TF_ZTF|_GX_TF_CTF)
#define 	GX_TF_A8   GX_CTF_A8
#define 	GX_TF_CI14   0xa
#define 	GX_TF_CI4   0x8
#define 	GX_TF_CI8   0x9
#define 	GX_TF_CMPR   0xE
#define 	GX_TF_I4   0x0
#define 	GX_TF_I8   0x1
#define 	GX_TF_IA4   0x2
#define 	GX_TF_IA8   0x3
#define 	GX_TF_RGB565   0x4
#define 	GX_TF_RGB5A3   0x5
#define 	GX_TF_RGBA8   0x6
#define 	GX_TL_IA8   0x00
#define 	GX_TL_RGB565   0x01
#define 	GX_TL_RGB5A3   0x02

#define 	GX_FALSE   0
#define 	GX_TRUE   1

// Used to flip textures data to big endian
#define SWAP_2(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )
#define SWAP_4(x) ( ((x) << 24) | \
         (((x) << 8) & 0x00ff0000) | \
         (((x) >> 8) & 0x0000ff00) | \
         ((x) >> 24) )
#define FLIP_ENDIAN16(x) (*(unsigned short *)&(x) = SWAP_2(*(unsigned short *)&(x)))
#define FLIP_ENDIAN32(x)   (*(unsigned int *)&(x)   = SWAP_4(*(unsigned int *)&(x)))

// Types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Path separator
extern char separator;

extern char logfile[MAXPATHLEN];

bool FileExist(const char *path);
bool DirExist(const char *path);

void ClearScreen();
void format_elapsed_time(char *time_str, double elapsed);

#endif
