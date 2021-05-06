/*
 *
 * @file
 * @author  Matthew Areno <mareno@sandia.gov>
 * @version	1.0
 *
 * @section  LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This file contains the base functions for performing all of the 
 * operations supported by this application, such as update, dec, 
 * parse, etc.  The first functions listed are support functions, 
 * followed by the 5 primary functions for implementing the previously
 * mentioned operations.
 */


#include <IMG3_Functions.h>

static int fd = 0;

/*! \fn		uint8_t * MapFileToMemory( const char *fileName, uint32_t *fileSize, uint32_t *mapSize )
	\brief	Opens the file provided and maps its contents to memory.
	\param	fileName 	(Input) 	The name of the file to be opened.
	\param	fileSize 	(Output)	The size of the file that was provided.
	\param	mapSize		(Output)	The size of the mapped area used for the file.
*/

static uint8_t * MapFileToMemory( const char *fileName, uint32_t *fileSize, uint32_t *mapSize )
{
	struct stat fileStat;
	uint8_t *data;
	int32_t pageSize = sysconf( _SC_PAGESIZE );

	ASSERT_RET( fileName, NULL );
	ASSERT_RET( fileSize, NULL );

	if ( fd != 0 ) {
		close( fd );
	}

	fd = open( fileName, O_RDONLY );
	if ( fd == -1 ) {
		PRINT_SYSTEM_ERROR();
		return NULL;
	}

	if ( fstat( fd, &fileStat ) == -1 ) {
		PRINT_SYSTEM_ERROR();
		close( fd );
		return NULL;
	}

 	*fileSize = fileStat.st_size;
	if ( *fileSize % pageSize  == 0 ) {
		*mapSize = ( ( *fileSize / pageSize ) + 1 ) * pageSize;
	} else {
		*mapSize = *fileSize;
	}

	data = (uint8_t *) mmap( NULL, *mapSize, PROT_READ, MAP_PRIVATE, fd, 0 );
	if ( data == MAP_FAILED ) {
		PRINT_SYSTEM_ERROR();
		close( fd );
		return NULL;
	}

	return data;
}

static void UnmapFileFromMemory( uint8_t *data, uint32_t mapSize )
{
	if ( data == NULL || mapSize == 0)
		return;

	munmap( data, mapSize );
	
	if ( fd != 0 ) {
		close( fd );
		fd = 0;
	}
}

static int32_t WriteDataToFile( const char *fileName, uint8_t *data, uint32_t dataLength )
{
	FILE *outputFile;

	ASSERT_RET( fileName, -1 );
	ASSERT_RET( data, -1 );
	ASSERT_RET( dataLength, -1 );

	fprintf( stdout, "Writing 0x%08x bytes of data to file %s...\r\n", dataLength, fileName );

	outputFile = fopen( fileName, "wb" );
	if ( outputFile == NULL ) {
		PRINT_SYSTEM_ERROR();
		return -1;
	}

	if ( fwrite( data, 1, dataLength, outputFile ) != dataLength ) {
		PRINT_SYSTEM_ERROR();
		return -1;
	}

	fclose( outputFile );
	return 0;
}

static uint8_t * 
EncryptIMG3Data( uint8_t *data, uint32_t length, char *deviceName, char *deviceVersion, char *section, uint8_t compress, uint8_t **encryptedData, uint32_t *encryptedLength ) 
{
	IMG3_OpensslInterface openssl;
	IMG3_LzssInterface lzss;
	IMG3_Sqlite3 sq;
	char *key = NULL, *iv = NULL;
	uint8_t *dataToEncrypt = NULL;
	size_t compressedLength;
	uint8_t allocatedLength = 0;

	ASSERT_RET( data, NULL );
	ASSERT_RET( deviceName, NULL );
	ASSERT_RET( deviceVersion, NULL );
	ASSERT_RET( section, NULL );

	if ( compress == 1 ) {
		fprintf( stdout, "Compressing 0x%08x bytes of data...\r\n", length );
		if ( lzss.LzssCompress( data, (size_t) length, &dataToEncrypt, &compressedLength) != 0) {
			goto EncryptIMG3Data_return;
		}
		length = compressedLength;
	} else {
		dataToEncrypt = data;
	}
		
	fprintf( stdout, "Retrieving key and iv...\r\n" );
	if (sq.RetrieveKeyAndIV(deviceName, deviceVersion, section, &key, &iv) != 0) {
		fprintf( stderr, "Unable to find key and iv for %s with version %s and section %s.\n", deviceName, deviceVersion, section);
		goto EncryptIMG3Data_free_compressed;
	}
	
	// Also, the data has to be padded before sending it to openssl.  This was originally done automatically
	// in the OpensslInterface code, but that requires the code to allocate yet another copy of the output.
	// Putting it here just seemed easier.
	if (length % 16 != 0) {
		*encryptedLength = length + (16 - (length % 16));
	} else {
		*encryptedLength = length;
	}

	// Now that we have our buffer, set the Key and IV values and decrypt the data.
	if ( openssl.SetKeyAndIV( key, iv ) ) {
		goto EncryptIMG3Data_free_compressed;
	}

	if ( openssl.EncryptData( dataToEncrypt, *encryptedLength, encryptedData ) ) {
		goto EncryptIMG3Data_free_compressed;
	}

	return *encryptedData;

EncryptIMG3Data_free_compressed:
	if ( compress )
		delete( dataToEncrypt );

EncryptIMG3Data_return:
	return NULL;
}

static uint8_t * 
DecryptIMG3Data( uint8_t *data, uint32_t length, char *deviceName, char *deviceVersion, char *section, uint8_t **decryptedData, uint32_t *decryptedLength ) 
{
	IMG3_OpensslInterface openssl;
	IMG3_LzssInterface lzss;
	IMG3_Sqlite3 sq;
	char *key = NULL, *iv = NULL;
	uint8_t *dataToDecrypt = NULL, *decompressedData = NULL;
	size_t decompressedLength;
	uint8_t allocatedLength = 0;

	ASSERT_RET( data, NULL );
	ASSERT_RET( deviceName, NULL );
	ASSERT_RET( deviceVersion, NULL );
	ASSERT_RET( section, NULL );

	fprintf( stdout, "Retrieving key and iv...\r\n" );
		
	// Before we can decrypt the data, we need the key and iv
	if (sq.RetrieveKeyAndIV(deviceName, deviceVersion, section, &key, &iv) != 0) {
		fprintf( stderr, "Unable to find key and iv for %s with version %s and section %s.\n", deviceName, deviceVersion, section);
		goto DecryptIMG3File_return;
	}

	printf( "Using: \n\tKey: %s\n\tIV: %s\n", key, iv );
	// Also, the data has to be padded before sending it to openssl.  This was originally done automatically
	// in the OpensslInterface code, but that requires the code to allocate yet another copy of the output.
	// Putting it here just seemed easier.
	if ( length % 16 != 0) {
		*decryptedLength = length + (16 - (length % 16));
		dataToDecrypt = new uint8_t[ *decryptedLength ];
		if ( dataToDecrypt == NULL ) {
			PRINT_SYSTEM_ERROR();
			goto DecryptIMG3File_return;
		}
		allocatedLength = 1;
		/***********************************************************************************************************************************
		************************************************************************************************************************************
		************************************************************************************************************************************
		*  EXCEPTIONALLY IMPORTANT!!!!!!
		*
		*  I THINK THIS IS PROBABLY A MISTAKE BY APPLE, BUT EVEN IF THE LENGTH OF THE DATA SECTION ISN'T AN EXACT MULTIPLE OF 16, THE 
		*  ADLER32 CHECKSUM REQUIRES A 16-BYTE ALIGNED BLOCK OF DATA.  FOR THAT REASON, EVEN THOUGH WHAT'S AT THE END MAY BE JUNK, WE STILL
		*  NEED TO READ IN AN EXACT MULTIPLE OF 16 BYTES WORTH OF DATA.
		*
		************************************************************************************************************************************
		************************************************************************************************************************************
		***********************************************************************************************************************************/
		memcpy( dataToDecrypt, data, *decryptedLength );
	} else {
		dataToDecrypt = data;
		*decryptedLength = length;
	}

	// Now that we have our buffer, set the Key and IV values and decrypt the data.
	if ( openssl.SetKeyAndIV(key, iv) ) 
		goto DecryptIMG3File_free_data;

	fprintf( stdout, "Decrypting data...\r\n" );
	if ( openssl.DecryptData(dataToDecrypt, *decryptedLength, decryptedData) )
		goto DecryptIMG3File_free_data;

	if ( lzss.IsFileCompressed( *decryptedData ) == 0 ) {
		fprintf( stdout, "Data appears to be compressed.  Decompressing 0x%08x bytes to data...\r\n", length );
		if ( lzss.LzssDecompress( *decryptedData, (size_t) *decryptedLength, &decompressedData, &decompressedLength ) ) 
			goto DecryptIMG3File_free_decrypted;
		
	//	delete ( *decryptedData );
		*decryptedData = decompressedData;
		*decryptedLength = decompressedLength;
	}

	if ( allocatedLength ) {
		delete( dataToDecrypt );
	}
	return *decryptedData;


DecryptIMG3File_free_decrypted:
	delete( *decryptedData );

DecryptIMG3File_free_data:
	if ( allocatedLength )
		delete( dataToDecrypt );

DecryptIMG3File_return:
	return NULL;
}

int32_t ListArchiveFiles( char *archiveFileName )
{
	IMG3_ZipInterface zip;
	list< char * > *files;
	list< char * >::iterator fileIt;

	ASSERT_RET( archiveFileName, -1 );

	files = zip.AnalyzeFile( archiveFileName );
	if (files == NULL) 
		return -1;

	fprintf( stdout, "Found the following files int he archive %s: \r\n", archiveFileName );

	for ( fileIt = files->begin(); fileIt != files->end(); ++fileIt) {
		fprintf( stdout, "\t%s\r\n", *fileIt);
	}

	return 0;
}

int32_t ExtractFileFromArchive( char *archiveFileName, char *section )
{
	IMG3_ZipInterface zip;
	list< char * > *files;
	list< char * >::iterator fileIt;

	ASSERT_RET( archiveFileName, -1 );

	files = zip.AnalyzeFile( archiveFileName );
	if ( files == NULL )
		return -1;

	if (section != NULL )
		zip.ExtractFiles( archiveFileName, section );
	else
		zip.ExtractAllFiles( archiveFileName );

	return 0;
}

int32_t UpdateIMG3Database( char *archiveFileName, char *deviceName, char *deviceVersion, char *deviceBuild )
{
	uint8_t *data;
	uint32_t fileSize, mapSize;
	IMG3_HtmlParser parser;
	IMG3_Sqlite3 sq;
	list< DecryptionInfo * > *decryptionInfo;

	data = MapFileToMemory( archiveFileName, &fileSize, &mapSize );
	if ( data == NULL ) {
		PRINT_SYSTEM_ERROR();
		goto UpdateIMG3Database_close_file;
	}

	parser.AddFilter( (char *) "a" );
	parser.AddFilter( (char *) "li" );
	parser.AddFilter( (char *) "span" );
	parser.AddFilter( (char *) "ul" );

	if ( !parser.ParseHtmlTags( data, fileSize ) ) {
		goto UpdateIMG3Database_unmap_file;
	}

	decryptionInfo = parser.GetDecryptionInfo();
	if (decryptionInfo == NULL ) {
		goto UpdateIMG3Database_unmap_file;
	}

	fprintf( stdout, "Updating database information...\r\n");
	sq.UpdateDatabase( deviceName, deviceBuild, deviceVersion, decryptionInfo );
	fprintf( stdout, "Completed database update.\r\n");
	UnmapFileFromMemory( data, mapSize );
	close( fd );
	return 0;
		
UpdateIMG3Database_unmap_file:
	UnmapFileFromMemory( data, mapSize );

UpdateIMG3Database_close_file:
	close( fd );

UpdateIMG3Database_return:
	return -1;
}

int32_t PatchKernelFile( char *archiveFileName, char *outputFileName, char *patchFileName, char *deviceName, char *deviceVersion ) {
	IMG3_ZipInterface zip;
	IMG3_FileInterface fileInterface;
	list< char * > *files, *extractedFiles;
	list< char * >::iterator fileIt;
	char section[] = "kernelcache";
	uint8_t allocatedList = 0;
	uint8_t *patchFileData = NULL, *data = NULL, *encryptedData = NULL, *decryptedData = NULL, *reencryptedData = NULL;
	uint32_t fileSize, mapSize;
	FILE *output = NULL, *patchFile = NULL;	
	uint32_t encryptedLength, decryptedLength, reencryptedLength;
	char lineBuffer[ 128 ];

	ASSERT_RET( archiveFileName, -1 );
	ASSERT_RET( patchFileName, -1 );
	ASSERT_RET( deviceName, -1 );
	ASSERT_RET( deviceVersion, -1 );

	/* First step: scan the archive provided for files that match the section kernelcache. */
	fprintf( stdout, "Scanning archive %s for %s files...\r\n", archiveFileName, section );
	files = zip.AnalyzeFile( archiveFileName );
	if ( files == NULL ) {
		/* The file may not be a zip archive.  For instance, the user may have provided an actual kernelcache file rather than
			a .ipsw file.  If that's the case, just add the file to the list and continue on. */
		if ( zip.GetError() != ZIP_ERROR_NOT_A_ZIP_FILE )
			goto PatchKernelFile_close_patch;
		extractedFiles = new list< char * >;
		if ( extractedFiles == NULL ) {
			PRINT_SYSTEM_ERROR();
			goto PatchKernelFile_return;
		}
		/* allocatedList just lets the application know if it needs to release memory allocated for extractedFiles before exiting. */
		allocatedList = 1;
		extractedFiles->push_back( archiveFileName );
	} else {
		fprintf( stdout, "Extracting all files for section %s...\r\n", section );
		extractedFiles = zip.ExtractFiles( archiveFileName, section );
		if ( extractedFiles == NULL )
			goto PatchKernelFile_close_patch;
	}

	/* Once we have our list of matching files, go through them one at a time applying the patches. */
	for ( fileIt = extractedFiles->begin(); fileIt != extractedFiles->end(); ++fileIt ) {
		if ( outputFileName != NULL ) {
			output = fopen( outputFileName, "wb" );
		} else {
			char generatedFileName[ 2049 ];
			snprintf( generatedFileName, 2048, "%s_patched", archiveFileName );
			output = fopen( generatedFileName, "wb" );
		}
		if ( output == NULL ) {
			PRINT_SYSTEM_ERROR();
			goto PatchKernelFile_return;
		}
	
		patchFile = fopen( patchFileName, "r" );
		if (patchFile == NULL ) {
			PRINT_SYSTEM_ERROR();
			goto PatchKernelFile_close_output;
		}

		fprintf( stdout, "Memory mapping the file %s in prep for decryption...\r\n", *fileIt );

		/* Before patching each file, map it to memory. */
		data = MapFileToMemory( *fileIt, &fileSize, &mapSize );
		if ( data == NULL )
			goto PatchKernelFile_delete_list;

		/* Since the original file is mapped to memory, we don't want to alter it.  Just make a copy. */
		patchFileData = new uint8_t[ mapSize ];
		if (patchFileData == NULL ) {
			PRINT_SYSTEM_ERROR();
			goto PatchKernelFile_unmap_file;
		}

		memset( patchFileData, 0, mapSize );
		memcpy( patchFileData, data, fileSize );

		/* Scan the file looking for the 'DATA' tag. */
		fprintf( stdout, "Retrieving data section...\r\n" );
		if ( fileInterface.ParseFile( patchFileData, fileSize ) )
			goto PatchKernelFile_unmap_file;

		//TBD: Fix this call
		//encryptedData = fileInterface.GetSectionData( IMG3_DATA );
		if (encryptedData == NULL )
			goto PatchKernelFile_unmap_file;

		//TBD: Fix this call
		//encryptedLength = fileInterface.GetSectionLength(IMG3_DATA);
		if (encryptedLength == 0) {
			goto PatchKernelFile_unmap_file;
		}
		
		/* Once we have the location of the data section, decrypt it and copy it's contents into decryptedData. */
		DecryptIMG3Data( encryptedData, encryptedLength, deviceName, deviceVersion, section, &decryptedData, &decryptedLength );
		if ( decryptedData == NULL ) 
			goto PatchKernelFile_unmap_file;

		fprintf( stdout, "Patching kernel...\r\n" );

		/* Then, one line at a time, apply our patches. */
		while (fgets(lineBuffer, 128, patchFile) != NULL) {
			uint32_t patchAddress, patchValue;
			uint32_t *tempPtr;
			
			/* The patch file should consist of two items per line: <patch_address> <patch_value>
				fgets will typically read a blank line at the end, so if we don't get two values from the line, just break out. */
			if ( sscanf(lineBuffer, "%x %x", &patchAddress, &patchValue) < 2 )
				break;
			if (patchAddress > decryptedLength) {
				fprintf(stdout,"The patch at address 0x%08x appears to be outside the file.\n", patchAddress);
				continue;
			}
			printf("Applying patch at address 0x%08x with value 0x%08x...\n", patchAddress, patchValue);
			tempPtr = (uint32_t *)(decryptedData + patchAddress);
			*tempPtr = patchValue;
		}

		fprintf( stdout, "Recompressing and encrypting data...\r\n" );

		/* Once we've patched the kernel, we need to recompress and reencrypt it. */
		EncryptIMG3Data( decryptedData, decryptedLength, deviceName, deviceVersion, section, 1, &reencryptedData, &reencryptedLength );
		if ( reencryptedData == NULL )
			goto PatchKernelFile_delete_decrypted;
		/* If all we've done is overwrite values in the kernel, our size should be the same. */
//		if ( encryptedLength != reencryptedLength ) {
//			fprintf( stdout, "Reencrypted data length is different.  Looks like we'll need to add support for this.\r\n" );
//			delete( reencryptedData );
//			delete( decryptedData );
//			UnmapFileFromMemory( data, mapSize );
//			fclose( patchFile );
//			fclose( output );
//			continue;
//			goto PatchKernelFile_delete_reencrypted;
//		}

		/* Overwrite the existing data section with our patched version. */
		memcpy(encryptedData, reencryptedData, reencryptedLength );
		
		if ( fwrite( patchFileData, 1, fileSize, output ) != fileSize ) {
			PRINT_SYSTEM_ERROR();
			goto PatchKernelFile_delete_reencrypted;
		}
		
		fprintf(stdout, "All data successfully written to file.\n");
		fclose( output );
		fclose( patchFile );
		UnmapFileFromMemory( data, mapSize );
		delete( decryptedData );
		delete( reencryptedData );
		decryptedData = NULL;
		reencryptedData = NULL;
	}
	if ( allocatedList == 1 )
		delete( extractedFiles );
	return 0;

PatchKernelFile_delete_reencrypted:
	delete( reencryptedData );

PatchKernelFile_delete_decrypted:
	delete( decryptedData );

PatchKernelFile_unmap_file:
	UnmapFileFromMemory( data, mapSize );

PatchKernelFile_delete_list:
	if ( allocatedList == 1 ) {
		fprintf( stdout, "attempting to deallocate extractedFiles.\r\n" );
		delete( extractedFiles );
	}

PatchKernelFile_close_patch:
	fclose( patchFile );

PatchKernelFile_close_output:
	fclose( output );

PatchKernelFile_return:
	return -1;
}

int32_t DecryptIMG3File( char *archiveFileName, char *outputFileName, char *deviceName, char *deviceVersion, char *section )
{
	IMG3_ZipInterface zip;
	IMG3_FileInterface fileInterface;
	IMG3_FileSection *fileSection;
	IMG3_FileSection dummySection;
	list< char * > *files, *extractedFiles;
	list< char * >::iterator fileIt;	
	uint8_t *data = NULL, *encryptedData = NULL, *decryptedData = NULL;
	uint32_t fileSize, mapSize, encryptedDataLength, decryptedLength;
	uint8_t allocatedList = 0;

	ASSERT_RET( archiveFileName, -1 );
	ASSERT_RET( deviceName, -1 );
	ASSERT_RET( deviceVersion, -1 );
	ASSERT_RET( section, -1 );

	fprintf( stdout, "Analyzing archive file %s...\r\n", archiveFileName );
	files = zip.AnalyzeFile( archiveFileName );
	if ( files == NULL ) {
		if ( zip.GetError() != ZIP_ERROR_NOT_A_ZIP_FILE )
			goto DecryptIMG3File_return;
		extractedFiles = new list< char * >;
		if ( extractedFiles == NULL ) {
			PRINT_SYSTEM_ERROR();
			goto DecryptIMG3File_return;
		}
		allocatedList = 1;
		extractedFiles->push_back( archiveFileName );
	} else {
		fprintf( stdout, "Extracting all files for section %s...\r\n", section );
		extractedFiles = zip.ExtractFiles( archiveFileName, section );
	}

	for ( fileIt = extractedFiles->begin(); fileIt != extractedFiles->end(); ++fileIt ) {
		fprintf( stdout, "Memory mapping the file %s in prep for decryption...\r\n", *fileIt );

		data = MapFileToMemory( *fileIt, &fileSize, &mapSize );
		if ( data == NULL )
			goto DecryptIMG3File_delete_list;

		fprintf( stdout, "Retrieving data section...\r\n" );
		if (fileInterface.ParseFile( data, fileSize ) )
			goto DecryptIMG3File_unmap_file;

		//TBD: Fix this call.
		//encryptedData = fileInterface.GetSectionData( IMG3_DATA );
		encryptedData = fileInterface.GetSectionData( IMG3_DATA );
		if ( encryptedData == NULL ) 
			goto DecryptIMG3File_unmap_file;
		//TBD: Fix this call.
		//encryptedDataLength = fileInterface.GetSectionLength( IMG3_DATA );
		encryptedDataLength = fileInterface.GetSectionDataLength( IMG3_DATA );
		if ( encryptedDataLength == 0 )
			goto DecryptIMG3File_unmap_file;

		WriteDataToFile( "data_section.bin", encryptedData, encryptedDataLength );

		DecryptIMG3Data( encryptedData, encryptedDataLength, deviceName, deviceVersion, section, &decryptedData, &decryptedLength );
		if (decryptedData == NULL )
			goto DecryptIMG3File_unmap_file;

		if ( outputFileName == NULL ) {
			char *fileName;
			uint32_t strLength = strlen( archiveFileName ) + strlen( "_decrypted" ) + 1;

			fileName = new char[ strLength ];
			if ( fileName == NULL ) {
				PRINT_SYSTEM_ERROR();
				goto DecryptIMG3File_delete_decrypted;
			}
			snprintf( fileName, strLength, "%s_decrypted", archiveFileName );
			WriteDataToFile( fileName, decryptedData, decryptedLength );
			delete ( fileName );
		} else {
			WriteDataToFile( outputFileName, decryptedData, decryptedLength );
		}

		delete( decryptedData );
		decryptedData = NULL;
		UnmapFileFromMemory( data, mapSize );
	}
	if ( allocatedList == 1 )
		delete( extractedFiles );
	return 0;

DecryptIMG3File_delete_decrypted:
	delete( decryptedData );

DecryptIMG3File_unmap_file:
	UnmapFileFromMemory( data, mapSize );

DecryptIMG3File_delete_list:
	if ( allocatedList == 1 )
		delete( extractedFiles );

DecryptIMG3File_return:
	return -1;
}

void ParseIMG3File( char *fileName )
{
	IMG3_FileInterface fileData;
	uint32_t fileSize, mapSize;
	uint8_t *data;

	data = MapFileToMemory( fileName, &fileSize, &mapSize );
	if ( data == NULL ) {
		PRINT_SYSTEM_ERROR();
		return;
	}

	fileData.ParseFile( data, fileSize );
	fileData.PrintSections();

	UnmapFileFromMemory( data, mapSize );
}

int32_t DecompressLZSSFile( char *fileName )
{
	IMG3_LzssInterface lzss;
	uint32_t fileSize, mapSize, decompressedLength;
	uint8_t *data = NULL;
	uint8_t *decompressedData = NULL;
	char *outputFileName;
	uint32_t strLength;


	data = MapFileToMemory( fileName, &fileSize, &mapSize );
	if( data == NULL ) {
		PRINT_SYSTEM_ERROR();
		return -1;
	}

	if ( lzss.IsFileCompressed( data ) == 0 ) {
		fprintf( stdout, "Data appears to be compressed.  Decompressing 0x%08x bytes to data...\r\n", fileSize );
		if ( lzss.LzssDecompress( data, (size_t) fileSize, &decompressedData, &decompressedLength ) ) {
			UnmapFileFromMemory( data, mapSize );
			return -1;
		}
	}

	strLength = strlen( fileName ) + strlen( "_decompressed" ) + 1;
	outputFileName = new char[ strLength ];
	if ( outputFileName == NULL ) {
		PRINT_SYSTEM_ERROR();
		UnmapFileFromMemory( data, mapSize );
		return -1;
	}
	snprintf( outputFileName, strLength, "%s_decompressed", fileName );
	WriteDataToFile( outputFileName, decompressedData, decompressedLength );
	delete ( outputFileName );

	UnmapFileFromMemory( data, mapSize );
	return 0;
}
