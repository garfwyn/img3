#ifndef __IMG3_FUNCTIONS_H_
#define __IMG3_FUNCTIONS_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <list>

#include "IMG3_defines.h"
#include "IMG3_typedefs.h"
#include "IMG3_includes.h"

using namespace std;

extern char img3SupportedFiles[][30];

int32_t PatchKernelFile( char *archiveFileName, char *outputFileName, char *patchFileName, char *deviceName, char *deviceVersion );
int32_t DecryptIMG3File( char *archiveFileName, char *outputFileName, char *deviceName, char *deviceVersion, char *section );
int32_t ListArchiveFiles( char *archiveFileName );
int32_t ExtractFileFromArchive( char *archiveFileName, char *section );
int32_t UpdateIMG3Database( char *archiveFileName, char *deviceName, char *deviceVersion, char *deviceBuild );
void ParseIMG3File( char *fileName );
int32_t DecompressLZSSFile( char *fileName );

#endif // __IMG3_FUNCTIONS_H_
