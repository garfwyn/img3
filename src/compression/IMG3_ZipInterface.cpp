/**
 * @file
 * @author Matthew Areno <engineereeyore@gmail.com>
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Implementation of all IMG3_ZipInterface class methods.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "IMG3_ZipInterface.h"

/*! \fn		IMG3_ZipInterface()
	\brief	Constructor for IMG3_ZipInterface class.
*/

IMG3_ZipInterface::IMG3_ZipInterface() 
{
	fd = NULL;
	errorCode = ZIP_ERROR_NONE;
	nodes.clear();
	files.clear();
}

/*!	\fn		~IMG3_ZipInterface()
	\brief	Deconstructor for IMG3_ZipInterface class.
*/

IMG3_ZipInterface::~IMG3_ZipInterface() 
{
	ResetData();
}

/*!	\fn		ResetData()
	\brief 	A private method used to reset the private member variables for the calling instance.
*/

void IMG3_ZipInterface::ResetData( void )
{
	list< char * >::iterator listIt;

	nodes.clear();
	for ( listIt = files.begin(); listIt != files.end(); ++listIt ) {
		char *name = *listIt;
		if ( name != NULL )
			delete( name );
	}
	files.clear();
	if ( fd != NULL )
		fclose( fd );

	errorCode = ZIP_ERROR_NONE;
}

/*!	\fn		AnalyzeFile( const char *fileName )
	\brief	A public method for analyzing ZIP compressed archives.  This function scans archives and creates a list of all files contained in the archive.
	\param	fileName pointer to a string containing the name of the archive to analyze
*/

list<char *> * IMG3_ZipInterface::AnalyzeFile(const char *fileName)
{
	ZIP_CentralDirectoryEnd end;
	long fileSize;
	char *ptr;

	CLASS_VALIDATE_PARAMETER( fileName, NULL );

	// Before analyzing a file, ensure all private member data has been reset.
	ResetData();

	memset(&end,0,sizeof(end));

	fd = fopen(fileName,"r");
	if (fd == NULL) {
		errorCode = errno;
		PRINT_SYSTEM_ERROR();
		return NULL;
	}

	// Central directories are located at the end of ZIP archive files, so move to the end and work backwards.
	if (fseek(fd,0,SEEK_END)) {
		errorCode = errno;
		PRINT_SYSTEM_ERROR();
		goto zip_getfilelist_close_error;
	}

	fileSize = ftell(fd);
	if (fileSize < 0) {
		errorCode = errno;
		PRINT_SYSTEM_ERROR();
		goto zip_getfilelist_close_error;
	}

	// Find the central directory end structure
	ptr = FindCentralDirectoryEnd(fd,fileSize);
	if (ptr == NULL) {
		errorCode = ZIP_ERROR_NOT_A_ZIP_FILE;
		PRINT_CLASS_ERROR( "no central directory end found" );
		goto zip_getfilelist_close_error;
	}

	memcpy(&end,ptr,sizeof(end) - CENTRAL_DIRECTORY_END_EXTRA);
	delete(ptr);
	end.comment = NULL;

	// Once we have the end structure, we can find the central directory listings
	if (ExtractCentralDirectoryListings(fd,&end) != 0) {
		errorCode = ZIP_ERROR_NO_CENTRAL_DIRECTORY_FOUND;
		PRINT_CLASS_ERROR( "no central directory was found" );
		goto zip_getfilelist_close_error;
	}
	// The central directory listing contain all the information we need, so once we have it return
	fclose(fd);
	return &files;

zip_getfilelist_close_error:
	fclose(fd);
	fd = NULL;
	return NULL;
}

/*!	\fn		PrintCentralDirectoryListing( ZIP_CentralDirectoryHeader *hdr )
	\brief	A private debug method used to print out the contents of the central directory header in a ZIP archive.
	\param	hdr	pointer to a ZIP_CentralDirectoryHeader structure that contains the information to print out.
*/

#ifdef IMG3_DEBUG

void IMG3_ZipInterface::PrintCentralDirectoryListing(ZIP_CentralDirectoryHeader *hdr)
{
	fprintf(stdout,"*******************************************************************************\n");
	fprintf(stdout,"*                         CENTRAL DIRECTORY RECORD                            *\n");
	fprintf(stdout,"*******************************************************************************\n");

	fprintf(stdout,"Signature: %#08x.\n",hdr->sig);
	fprintf(stdout,"Version zip was made with: %u.\n",hdr->versionMade);
	fprintf(stdout,"Version required: %u.\n",hdr->versionNeeded);
	fprintf(stdout,"Flags: %#04x.\n",hdr->flags);
	fprintf(stdout,"Compression Method: %#04x.\n",hdr->compressionMethod);
	fprintf(stdout,"Last Modification Time: %u.\n",hdr->lastModTime);
	fprintf(stdout,"Last Modification Date: %u.\n",hdr->lastModDate);
	fprintf(stdout,"crc32: %#08x.\n",hdr->crc32);
	fprintf(stdout,"Compressed Size: %u.\n",hdr->compressedSize);
	fprintf(stdout,"Uncompressed Size: %u.\n",hdr->uncompressedSize);
	fprintf(stdout,"File Name Length: %u.\n",hdr->fileNameLength);
	fprintf(stdout,"Extra Field Length: %u.\n",hdr->extraFieldLength);
	fprintf(stdout,"File Comment Length: %u.\n",hdr->fileCommentLength);
	fprintf(stdout,"Starting Disk Number: %u.\n",hdr->startDiskNumber);
	fprintf(stdout,"Internal File Attributes: %u.\n",hdr->internalFileAttr);
	fprintf(stdout,"External File Attributes: %u.\n",hdr->externalFileAttr);
	fprintf(stdout,"File Header Offset: %#08x.\n",hdr->fileHeaderOffset);
	fprintf(stdout,"File Name: %s.\n",hdr->fileName);
}

/*!	\fn		PrintCentralDirectoryEnd( ZIP_CentralDirectoryEnd *ptr )
	\brief	A private debug method used to print out the contents of the central directory end header in a ZIP archive.
	\param	ptr pointer to a ZIP_CentralDirectoryEnd structure that contains the information to print out.
*/

void IMG3_ZipInterface::PrintCentralDirectoryEnd(ZIP_CentralDirectoryEnd *ptr)
{
	fprintf(stdout,"*******************************************************************************\n");
	fprintf(stdout,"*                         CENTRAL DIRECTORY END                               *\n");
	fprintf(stdout,"*******************************************************************************\n");

	fprintf(stdout,"Signature: %#08x.\n",ptr->sig);
	fprintf(stdout,"Number on this disk: %u.\n",ptr->numberOnDisk);
	fprintf(stdout,"Disk where central directory starts: %u.\n",ptr->centralDirectoryDisk);
	fprintf(stdout,"Number of central directory records on disk: %u.\n",ptr->centralDirectoryNumOnDisk);
	fprintf(stdout,"Total number of central directory records: %u.\n",ptr->centralDirectoryTotalNum);
	fprintf(stdout,"Size of central directory: %#08x (%u).\n",ptr->centralDirectorySize,ptr->centralDirectorySize);
	fprintf(stdout,"Offset to start of central directory: %#08x.\n",ptr->centralDirectoryOffset);
	fprintf(stdout,"ZIP file comment length: %u bytes.\n",ptr->commentLength);
}

#endif

/*!	\fn		FindCentralDirectoryEnd( FILE *fd, long fileSize )
	\brief	A private function used to determine the starting location of the central directory end structure inside a ZIP archive.
	\param	fd pointer to the ZIP archive file to examine
	\param	fileSize size of the ZIP archive in bytes
*/

char * IMG3_ZipInterface::FindCentralDirectoryEnd(FILE *fd, long fileSize)
{
	size_t result;
	uint16_t index;
	uint32_t marker;
	long bytesToRead = ZIP_BUFFER_SIZE;
	char *endBuffer;

	errorCode = ZIP_ERROR_NONE;

	CLASS_VALIDATE_PARAMETER( fd, NULL );
	CLASS_VALIDATE_PARAMETER( fileSize, NULL );

	if (fileSize < ZIP_BUFFER_SIZE)
		bytesToRead = fileSize;

	// ZIP comments, and the end structure, should never be more than 1024 bytes, so read in that many
	if (fseek(fd,-bytesToRead,SEEK_END)) {
		errorCode = errno;
		PRINT_SYSTEM_ERROR();
		return NULL;
	}
	result = fread(buffer,bytesToRead,1,fd);
	if (result == 0) {
		errorCode = errno;
		PRINT_SYSTEM_ERROR();
		return NULL;
	}

	// Scan through the bytes read in looking for the central directory end marker
	for (index = 0; index < bytesToRead - 3; index++) {
			marker = *((uint32_t *)(buffer+index));
			if (marker == CENTRAL_DIRECTORY_END_MARKER) {
				endBuffer = new char[bytesToRead - index];
				if (endBuffer == NULL) {
					errorCode = errno;
					PRINT_SYSTEM_ERROR();
					return NULL;
				}
				// Once the end marker is found, copy of the end header and return
				memcpy(endBuffer,buffer+index,bytesToRead-index);
				return endBuffer;
			}
	}
	// If no end marker is found, report the error
	errorCode = ZIP_ERROR_NOT_A_ZIP_FILE;
	PRINT_CLASS_ERROR( "no end directory marker found" );
	return NULL;
}

/*!	\fn		ExtractCentralDirectoryListing( FILE *fd, ZIP_CentralDirectoryEnd *end )
	\brief	A private method used to extract a list of all files and file nodes contained within the specified ZIP archive file
	\param	fd pointer to the ZIP archive file structure
	\param	end pointer to the ZIP_CentralDirectoryEnd structure for the specified file
*/

int32_t IMG3_ZipInterface::ExtractCentralDirectoryListings(FILE *fd, ZIP_CentralDirectoryEnd *end)
{
	char *records;
	ZIP_CentralDirectoryHeader dirHeader;
	uint16_t index, sizeOfStruct;
	uint32_t currOffset, recordSize;
	ZIP_FileNode *node;

	errorCode = ZIP_ERROR_NONE;
	
	CLASS_VALIDATE_PARAMETER( fd, -1 );
	CLASS_VALIDATE_PARAMETER( end, -1 );

	sizeOfStruct = sizeof(dirHeader);

	// Using the information in the central directory end structure, allocate room for the central directory and read it in
	records = new char[end->centralDirectorySize];
	if (!records) {
		errorCode = errno;
		PRINT_SYSTEM_ERROR();
		return -1;
	}
	if(fseek(fd,end->centralDirectoryOffset,SEEK_SET)) {
		errorCode = errno;
		PRINT_SYSTEM_ERROR();
		return -1;
	}
	if (fread(records,end->centralDirectorySize,1,fd) == 0) {
		errorCode = errno;
		PRINT_SYSTEM_ERROR();
		return -1;
	}

	
	currOffset = recordSize = 0;
	for (index = 0; index < end->centralDirectoryTotalNum; index++) {
		// For each entry in the central directory, create a file node and extract its information
		memset(&dirHeader,0,sizeOfStruct);
		memcpy(&dirHeader,records+currOffset,sizeOfStruct - CENTRAL_DIRECTORY_HEADER_EXTRA);
		if (dirHeader.sig != CENTRAL_DIRECTORY_HEADER_MARKER) {
			fprintf(stderr,"%s: Should be a record at offset %#08x, but the marker isn't there.\n", __FUNCTION__, end->centralDirectoryOffset+currOffset);
			goto traversal_error;
		}
		node = new ZIP_FileNode;
		if (node == NULL) {
			errorCode = errno;
			PRINT_SYSTEM_ERROR();
			goto traversal_error;
		}
		node->offset = dirHeader.fileHeaderOffset;
		node->startingDisk = dirHeader.startDiskNumber;
		node->compressedSize = dirHeader.compressedSize;
		node->uncompressedSize = dirHeader.uncompressedSize;
		if (dirHeader.fileNameLength != 0) {
			dirHeader.fileName = new uint8_t[dirHeader.fileNameLength+1];
			if (!dirHeader.fileName) {
				errorCode = errno;	
				PRINT_SYSTEM_ERROR();
			}
			node->fileName = new char[dirHeader.fileNameLength+1];
			if (!node->fileName) {
				errorCode = errno;
				PRINT_SYSTEM_ERROR();
				goto traversal_error;
			}
			memcpy(dirHeader.fileName,records+currOffset+sizeOfStruct-CENTRAL_DIRECTORY_HEADER_EXTRA,dirHeader.fileNameLength);
			dirHeader.fileName[dirHeader.fileNameLength] = '\0';
			strncpy(node->fileName,(char *)dirHeader.fileName,dirHeader.fileNameLength);
		}
		nodes.push_back(node);
		files.push_back(node->fileName);

		recordSize = sizeOfStruct - CENTRAL_DIRECTORY_HEADER_EXTRA + dirHeader.fileNameLength + dirHeader.extraFieldLength + dirHeader.fileCommentLength;
		currOffset += recordSize;
		if (currOffset > end->centralDirectorySize) {
			goto traversal_error;
		}
	}

	delete(records);
	return 0;

traversal_error:
	nodes.clear();
	delete(records);
	if (node != NULL)
		delete(node);
	return -1;
}

/*!	\fn		ExtractFiles( const char *archiveName, char *section )
	\brief	A public method used to extract all files from a ZIP archive whose name includes the section string provided
	\param	archiveName	pointer to a string buffer containing the name of the ZIP archive
	\param 	section pointer to a string buffer containing the section name to use as a filter
*/

list<char *> * IMG3_ZipInterface::ExtractFiles(const char *archiveName, char *section)
{
	list<ZIP_FileNode *>::iterator listIt;
	list<char *> *matchingFiles;
	ZIP_FileNode *node;
	char *fileName = NULL;
	pid_t pid;
	int status;

	errorCode = ZIP_ERROR_NONE;

	CLASS_VALIDATE_PARAMETER( section, NULL );
	CLASS_VALIDATE_PARAMETER( archiveName, NULL );

	matchingFiles = new list <char *>;
	if (matchingFiles == NULL) {
		errorCode = errno;
		PRINT_SYSTEM_ERROR();
		return NULL;
	}

	// Next, we need to figure out what file they actually want extracted.
	for (listIt = nodes.begin(); listIt != nodes.end(); ++listIt) {
		node = *listIt;
		// Iterate through all files in the archive looking for ones that contain the section string in their name
		if (strcasestr(node->fileName,section) != NULL) {
			// If a match occurs, store the file name and extract it from the ZIP archive
			fileName = node->fileName;
			matchingFiles->push_back(fileName);
			pid = vfork();
			if (pid == 0) {
				execlp("/usr/bin/unzip", "unzip", "-o", "-q", archiveName, fileName, NULL);
			} else if (pid > 0){
				if (wait(&status) == -1) {
					errorCode = errno;
					PRINT_SYSTEM_ERROR();
					return NULL;
				}
				fprintf(stdout,"Successfully extracted the file %s from the archive %s.\n", fileName, archiveName);
			} else {
				errorCode = errno;
				PRINT_SYSTEM_ERROR();
				return NULL;
			}
		}
	}

	// Return a list of all files extracted from the ZIP archive provided.
	return matchingFiles;
}

/*!	\fn		ExtractAllFiles( const char *archiveName )
	\brief	A public method used to extract all the files contained within the ZIP archive specified.
	\param	archiveName pointer to a string buffer containing the name of the ZIP archive
*/

int32_t IMG3_ZipInterface::ExtractAllFiles(const char *archiveName)
{
	pid_t pid;
	int status;

	errorCode = ZIP_ERROR_NONE;

	CLASS_VALIDATE_PARAMETER( archiveName, -1 );

	// Use the unzip system application to extract all the files contained within the specified ZIP archive.
	pid = vfork();
	if (pid == 0) {
		execlp("/usr/bin/unzip", "unzip", "-q", archiveName, NULL);
	} else if (pid > 0) {
		if (waitpid(pid, &status, 0) == -1) {
			errorCode = errno;
			PRINT_SYSTEM_ERROR();
			return -1;
		}
		fprintf(stdout,"Successfully unzipped the archive %s.\n", archiveName);
	} else {
		errorCode = errno;
		PRINT_SYSTEM_ERROR();
		return -1;
	}
	return 0;
}
