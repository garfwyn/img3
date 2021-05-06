/*!
	\file IMG3_ZipInterface.h
 	\author Matthew Areno
 	\version 1.0

	This is a C++ class for interacting with the ZIP compressed files.  It does not support a full implementation 
	of the ZIP compression specification, but is rather used to extract information about a ZIP compressed file.
	All calls to decompression methods simply wrap to a system call to the program 'zip'.
*/

#ifndef IMG3_ZIPINTERFACE_H_
#define IMG3_ZIPINTERFACE_H_

#include <stdint.h>
#include <list>
#include "IMG3_defines.h"
#include "IMG3_typedefs.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

using namespace std;

#define LOCAL_FILE_HEADER_MARKER		0x04034b50
#define CENTRAL_DIRECTORY_HEADER_MARKER	0x02014b50
#define CENTRAL_DIRECTORY_END_MARKER	0x06054b50

// These represent the number of possible extra pointers for each structure.  For instance,
// a central directory end structure doesn't have to have any comments, so that extra
// byte in the structure may not exist in the file.  That being the case, we don't want
// to memcpy more than is actually there
#define LOCAL_FILE_HEADER_EXTRA			(2*sizeof(uint8_t*))
#define CENTRAL_DIRECTORY_HEADER_EXTRA	(3*sizeof(uint8_t*))
#define CENTRAL_DIRECTORY_END_EXTRA		(1*sizeof(uint8_t*))
#define ZIP_BUFFER_SIZE 1024

#define ZIP_ERROR_NONE											0x0000
#define ZIP_ERROR_SYSTEM										0x0001
#define ZIP_ERROR_MALFORMED_LIST								0x0002
#define ZIP_ERROR_INDEX_GREATER_THAN_SIZE						0x0003
#define ZIP_ERROR_NO_CENTRAL_DIRECTORY_FOUND					0x0004
#define ZIP_ERROR_EXTRACTING_CENTRAL_DIRECTORY_LISTINGS_FAILED	0x0005
#define ZIP_ERROR_NOT_A_ZIP_FILE								0x0006

#define CHUNK 16384

/**
 * A structure representing individual file nodes inside of a ZIP archive.
 */

typedef struct ZIP_FileNode {
	uint16_t	startingDisk;
	uint32_t	offset;
	uint32_t	compressedSize;
	uint32_t	uncompressedSize;
	char 		*fileName;
} ZIP_FileNode;

/**
 * A structure representing the local header inside of a ZIP archive.
 */

typedef struct ZIP_LocalHeader {
	uint32_t	sig;
	uint16_t	versionNeeded;
	uint16_t	flags;
	uint16_t	compressionMethod;
	uint16_t	lastModTime;
	uint16_t	lastModDate;
	uint32_t	crc32;
	uint32_t	compressedSize;
	uint32_t	uncompressedSize;
	uint16_t	fileNameLength;
	uint16_t	extraFieldLength;
	uint8_t		*fileName;
	uint8_t		*extra;
}__attribute__((__packed__)) ZIP_LocalHeader;

/**
 * A structure representing the central directory header inside a ZIP archive.
 */

typedef struct ZIP_CentralDirectoryHeader {
	uint32_t	sig;
	uint16_t	versionMade;
	uint16_t	versionNeeded;
	uint16_t	flags;
	uint16_t	compressionMethod;
	uint16_t	lastModTime;
	uint16_t	lastModDate;
	uint32_t	crc32;
	uint32_t	compressedSize;
	uint32_t	uncompressedSize;
	uint16_t	fileNameLength;
	uint16_t	extraFieldLength;
	uint16_t	fileCommentLength;
	uint16_t	startDiskNumber;
	uint16_t	internalFileAttr;
	uint32_t	externalFileAttr;
	uint32_t	fileHeaderOffset;
	uint8_t		*fileName;
	uint8_t		*extra;
	uint8_t		*fileComment;
}__attribute__((__packed__)) ZIP_CentralDirectoryHeader;

/**
 * A structure representing the central directory end inside an ZIP archive.
 */

typedef struct ZIP_CentralDirectoryEnd {
	uint32_t	sig;
	uint16_t	numberOnDisk;
	uint16_t	centralDirectoryDisk;
	uint16_t	centralDirectoryNumOnDisk;
	uint16_t	centralDirectoryTotalNum;
	uint32_t	centralDirectorySize;
	uint32_t	centralDirectoryOffset;
	uint16_t	commentLength;
	uint8_t		*comment;
}__attribute__((__packed__)) ZIP_CentralDirectoryEnd;

/*! IMG3_ZipInterface class */

class IMG3_ZipInterface {
private:
	FILE *fd;						//!< File descriptor representing the archive.
	char buffer[ZIP_BUFFER_SIZE+1];	//!< A temporary buffer for storing file data.
	list<ZIP_FileNode *> nodes;		//!< A list of all file nodes contained within the archive.
	list<char *> files;				//!< A list of all files contained within the archive.
	int32_t errorCode;				//!< An error code value representing any error that might occur.

	char * FindCentralDirectoryEnd(FILE *,long);  //!< A private function for determining the location of the central directory end.
	int32_t ExtractCentralDirectoryListings(FILE *fd, ZIP_CentralDirectoryEnd *);  //!< A private function used to extract all central directory information.

#ifdef IMG3_DEBUG
	void PrintCentralDirectoryListing(ZIP_CentralDirectoryHeader *hdr); //!< A private debug function for printing out central directory listings.
	void PrintCentralDirectoryEnd(ZIP_CentralDirectoryEnd *ptr);	//!< A private debug function for printing out central directory end information.
#endif
	void ResetData(); //!< A private function for resetting all private variable in the class.

public:
	IMG3_ZipInterface();	//!< A public constructor for the IMG3_ZipInterface class.
	virtual ~IMG3_ZipInterface(); //!< A public deconstructor for the IMG3_ZipInterface class.

	list<char *> * AnalyzeFile(const char *fileName);	// A public method for analyzing ZIP archives to determine files contained therein.
	list<char *> * ExtractFiles(const char *archiveName, char *section); // A public method for extracting single files from a ZIP archive.
	int32_t ExtractAllFiles(const char *archiveName); // A public method for extracting all files contained in the specified ZIP archive.
	int32_t GetError( void ) { return errorCode; }	// A public method for retrieving the last error message the class instance encountered.
};

#endif /* IMG3_ZIPINTERFACE_H_ */
